if [ $# -eq 0 ]; then
    COMMIT_MESSAGE="master: RGR 4"
else
    COMMIT_MESSAGE="master: $1"
fi

git add .
git commit -m "$COMMIT_MESSAGE"
git push