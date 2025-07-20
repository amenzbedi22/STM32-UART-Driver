import os
import requests
from datetime import datetime, timedelta, timezone
from dateutil.parser import parse
from collections import defaultdict

# === CONFIGURATION ===
GITHUB_TOKEN = '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'  # Replace with your token or use env variable
REPO = 'amenzbedi22/STM32-UART-Driver'
OWNER = 'amenzbedi22'
PROJECT_NUMBER = 4  # GitHub Projects V2 number

headers = {
    "Authorization": f"token {GITHUB_TOKEN}",
    "Accept": "application/vnd.github+json"
}

now = datetime.now(timezone.utc)
since = (now - timedelta(days=7)).isoformat()

stats = {
    "add": 0,
    "fix": 0,
    "refactor": 0,
    "other_commits": 0,
    "issues_opened": 0,
    "issues_closed": 0,
    "bug_issues_closed": 0,
    "tasks_completed": 0
}

contributors = defaultdict(lambda: {
    'kpi': 0,
    'bug_fixes': 0,
    'total_actions': 0,
    'add_commits': 0,
    'refactor_commits': 0,
    'active_days': set()
})

def get_commits():
    url = f'https://api.github.com/repos/{REPO}/commits'
    params = {'since': since}
    response = requests.get(url, headers=headers, params=params)
    commits = response.json()

    for commit in commits:
        msg = commit['commit']['message'].lower()
        author = commit['commit']['author']['name'] if commit['commit']['author'] else "Unknown"
        commit_date = parse(commit['commit']['author']['date']).date()

        if 'fix' in msg:
            stats['fix'] += 1
            contributors[author]['kpi'] += 2.5
            contributors[author]['bug_fixes'] += 1
        elif 'add' in msg or 'enhancement' in msg:
            stats['add'] += 1
            contributors[author]['kpi'] += 2
            contributors[author]['add_commits'] += 1
        elif 'refactor' in msg:
            stats['refactor'] += 1
            contributors[author]['refactor_commits'] += 1
        else:
            stats['other_commits'] += 1
            contributors[author]['kpi'] += 1

        contributors[author]['total_actions'] += 1
        contributors[author]['active_days'].add(commit_date)

def get_issues():
    url = f'https://api.github.com/repos/{REPO}/issues'
    params = {'state': 'all', 'since': since}
    response = requests.get(url, headers=headers, params=params)
    issues = response.json()

    for issue in issues:
        created = parse(issue['created_at'])
        closed_at = parse(issue['closed_at']) if issue.get('closed_at') else None
        labels = [label['name'].lower() for label in issue.get('labels', [])]

        if created >= now - timedelta(days=7):
            stats['issues_opened'] += 1

        if closed_at and closed_at >= now - timedelta(days=7):
            stats['issues_closed'] += 1
            if 'bug' in labels:
                stats['bug_issues_closed'] += 1
            author = issue['user']['login']
            contributors[author]['kpi'] += 2
            contributors[author]['total_actions'] += 1

def get_project_tasks():
    graphql_url = "https://api.github.com/graphql"
    graphql_query = f"""
    {{
      user(login: "{OWNER}") {{
        projectV2(number: {PROJECT_NUMBER}) {{
          items(first: 50) {{
            nodes {{
              content {{
                __typename
                ... on Issue {{
                  title
                  updatedAt
                }}
                ... on PullRequest {{
                  title
                  updatedAt
                }}
                ... on DraftIssue {{
                  title
                  updatedAt
                }}
              }}
              updatedAt
              fieldValues(first: 10) {{
                nodes {{
                  ... on ProjectV2ItemFieldSingleSelectValue {{
                    name
                  }}
                  ... on ProjectV2ItemFieldTextValue {{
                    text
                  }}
                }}
              }}
            }}
          }}
        }}
      }}
    }}
    """

    response = requests.post(graphql_url, json={'query': graphql_query}, headers=headers)
    data = response.json()

    try:
        items = data['data']['user']['projectV2']['items']['nodes']
        for item in items:
            updated = parse(item['updatedAt'])
            status_list = [
                field.get('name', '').lower() or field.get('text', '').lower()
                for field in item['fieldValues']['nodes']
            ]
            if updated >= now - timedelta(days=7):
                if any(status in ['done', 'completed'] for status in status_list):
                    stats['tasks_completed'] += 1
    except Exception as e:
        print(" Error fetching project tasks:", e)

def calculate_kpi():
    return (
        (2 * stats['add']) +
        (2.5 * stats['fix']) +
        (2 * stats['issues_closed']) +
        (3 * stats['tasks_completed'])
    )

def assign_badges():
    badges = defaultdict(list)

    if not contributors:
        return badges

    top = max(contributors.items(), key=lambda x: x[1]['kpi'], default=None)
    if top:
        badges[top[0]].append("Top Contributor")

    bug = max(contributors.items(), key=lambda x: x[1]['bug_fixes'], default=None)
    if bug and bug[1]['bug_fixes'] > 0:
        badges[bug[0]].append("Bug Squasher")

    active = max(contributors.items(), key=lambda x: x[1]['total_actions'], default=None)
    if active:
        badges[active[0]].append("Most Active")

    feature = max(contributors.items(), key=lambda x: x[1]['add_commits'], default=None)
    if feature and feature[1]['add_commits'] > 0:
        badges[feature[0]].append("Feature Creator")

    refactor = max(contributors.items(), key=lambda x: x[1]['refactor_commits'], default=None)
    if refactor and refactor[1]['refactor_commits'] > 0:
        badges[refactor[0]].append("Code Refactorer")

    for contributor, data in contributors.items():
        if len(data['active_days']) >= 3:
            badges[contributor].append("Consistent Contributor")

    return badges

def generate_prompt():
    kpi_score = calculate_kpi()
    badges = assign_badges()

    date_range = f"{(now - timedelta(days=7)).date()} to {now.date()}"

    badges_text = "\n".join(
        f"- {contrib}: {', '.join(badge_list)}"
        for contrib, badge_list in badges.items()
    ) or "No badges assigned"

    previous_kpi = 20

    # Compose individual contributors KPI and badges section
    contributors_text = ""
    for contributor, data in contributors.items():
        contrib_badges = badges.get(contributor, ["None"])
        contributors_text += f"- **{contributor}**: KPI Score = {data['kpi']:.2f}, Badges = {', '.join(contrib_badges)}\n"

    prompt_text = f"""
You are an expert project manager generating a weekly GitHub repository report.

Report Structure:
• Header with the date range (week)
• Technical achievements (based on commits)
• Bugs resolved (closed issues of type bug)
• Backlog tasks (from GitHub Projects)
• KPI Score and assigned badges

Summary of the past week ({date_range}):
- Commits Added: {stats['add']}
- Commits Fixed: {stats['fix']}
- Refactor Commits: {stats['refactor']}
- Other Commits: {stats['other_commits']}

- Issues Opened: {stats['issues_opened']}
- Issues Closed: {stats['issues_closed']}

- Tasks Completed from Backlog: {stats['tasks_completed']}

Contributor Badges:
{badges_text}

Individual Contributor KPI Scores and Badges:
{contributors_text}

KPI Scores:
- Current Week KPI: {kpi_score}
- Previous Week KPI: {previous_kpi}

Please generate a professional, concise, and clear weekly report in Markdown format following the above report structure.
"""
    return prompt_text.strip()

if __name__ == "__main__":
    get_commits()
    get_issues()
    get_project_tasks()
    prompt = generate_prompt()
    
    # Save prompt to a text file
    with open("prompt.txt", "w", encoding="utf-8") as f:
        f.write(prompt)
    
    print("Weekly report prompt saved to prompt.txt")
