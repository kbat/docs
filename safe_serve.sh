git branch safe_serve
git checkout safe_serve

gitbook install && gitbook bulid
gitbook serve

git checkout master
git branch -d safe_serve
