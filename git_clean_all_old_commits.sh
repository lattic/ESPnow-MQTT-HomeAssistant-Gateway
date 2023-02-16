!#/bin/bash
#checkout
git checkout --orphan latest_branch
#add all files
git add -A
#commit all changes - give here proper message
git commit -am "initial commit"
#delete the branch main
git branch -D main
#rename latest_branch to main
git branch -m main
#force update your repository to github
git push -f origin main
