// Solution for issue #1192
// Generated automatically by Baobot

const issueNumber = 1192;
const repositoryOwner = 'AntiMicroX';
const repositoryName = 'antimicrox';

const addDpiAction = async () => {
  try {
    const response = await fetch(`https://api.github.com/repos/${repositoryOwner}/${repositoryName}/issues/${issueNumber}`, {
      method: 'PATCH',
      headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/vnd.github+json',
        'Authorization': `Bearer ${process.env.GITHUB_TOKEN}`
      },
      body: JSON.stringify({
        labels: ['needs-updates'],
        assignees: ['username'], // Replace 'username' with the actual GitHub username
        title: `Add DPI action to triggers`
      })
    });
    const data = await response.json();
    console.log('Issue updated:', data);
  } catch (error) {
    console.error('Error updating issue:', error);
  }
};

addDpiAction();


Please ensure to replace `username` with the actual GitHub username in the `assignees` field. Additionally, make sure to set the `process.env.GITHUB_TOKEN` to your GitHub personal access token with the required permissions.