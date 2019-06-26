#
# SPDX-License-Identifier: Zyxel
# (C) 2017 Sphairon GmbH (a ZyXEL company)
#

GIT_USER_EMAIL='gemini-templates@sphairon.com'
GIT_USER_NAME='gemini-templates unit-test'

git_cmd() {
    echo "> git " "$@"
    LC_ALL=C git "$@"
}

git_repo_setup() {
    git_cmd config user.email "$GIT_USER_EMAIL"
    git_cmd config user.name "$GIT_USER_NAME"
}

git_repo_init_empty() {
    local -r git_dir=$1

    mkdir -p "$git_dir"
    pushd "$git_dir"

    git_cmd init
    git_repo_setup
    git_cmd commit --allow-empty -m "Root commit"
    popd
}

git_repo_create() {
    local -r root_dir=$1
    local -r name=$2
    local git_bare_dir=$root_dir/scm/${name}.git
    local git_wc_dir=$root_dir/wc/${name}
    local git_hashes_master=$root_dir/scm/${name}.master.hashes
    local git_hashes_next=$root_dir/scm/${name}.next.hashes

    # Create bare repo
    mkdir -p "$git_bare_dir"
    pushd "$git_bare_dir"
    git_cmd init --bare
    popd

    # Create clone of repo
    git_repo_init_empty "$git_wc_dir"

    pushd "$git_wc_dir"
    git_cmd remote add origin "$git_bare_dir"
    git_cmd remote update
    git_cmd tag root
    git_cmd checkout -b next
    touch .gitignore
    git_cmd add .gitignore
    git_cmd commit -m "Add .gitignore"
    for i in $(seq 0 1 2); do
        echo "feature $i" >> foo.txt
        git_cmd add foo.txt
        git_cmd commit -m "feature $i"
    done
    git_cmd checkout master
    git_cmd merge --no-ff next
    git_cmd tag -m "release 1.0" v1.0
    git_cmd log --oneline --no-abbrev-commit master > "$git_hashes_master"
    git_cmd checkout next
    for i in $(seq 6 1 8); do
        echo "feature $i" >> foo.txt
        git_cmd add foo.txt
        git_cmd commit -m "feature $i"
    done
    git_cmd push origin -u root master next v1.0
    git_cmd log --oneline --no-abbrev-commit next > "$git_hashes_next"
    popd

    cat "$git_hashes_master"
    cat "$git_hashes_next"
}

git_repo_url() {
    local -r root_dir=$1
    local -r name=$2

    echo "file://$root_dir/scm/${name}.git"
}

git_repo_hashes() {
    local -r root_dir=$1
    local -r name=$2
    local -r branch=$3

    echo "$root_dir/scm/${name}.${branch}.hashes"
}

git_repo_clone() {
    local -r git_uri=$1
    local -r dest_dir=$2

    git_cmd clone "$git_uri" "$dest_dir"
    pushd "$dest_dir"
    git_repo_setup
    popd
}

# Fail and display path of the Git repo if it is not a valid repo.
assert_git_repo_is_valid() {
    local -r repo_path=$1
    local -i err=0

    if [ -d "$repo_path" ]; then
        pushd "$repo_path" > /dev/null
        git rev-parse --git-dir &> /dev/null || err=1
        popd > /dev/null
    else
        err=1
    fi

    if (( err )); then
        batslib_print_kv_single 10 'Path' "$repo_path" \
            | batslib_decorate 'Git repo is not valid, but it was expected to be' \
            | fail
    fi
}

assert_git_repo_is_clean() {
    if [ "$(git status --porcelain --untracked-files=no)" != "" ]; then
        git status --porcelain --untracked-files=no >&2
        echo "repo is dirty" \
            | batslib_decorate 'ERROR: assert_git_repo_is_clean' \
            | fail
    fi
}

assert_git_repo_is_dirty() {
    if [ "$(git status --porcelain --untracked-files=no)" = "" ]; then
        git status --porcelain --untracked-files=no >&2
        echo "repo is clean" \
            | batslib_decorate 'ERROR: assert_git_repo_is_dirty' \
            | fail
    fi
}

assert_git_branch_current() {
    local -r exp_branch=$1
    local -r cur_branch="$(git symbolic-ref -q --short HEAD)"

    if [ "$exp_branch" != "$cur_branch" ]; then
        batslib_print_kv_single_or_multi 8 \
            'expected' "$exp_branch" \
            'actual'   "$cur_branch" \
            | batslib_decorate 'Current branch is not equal' \
            | fail
    fi
}

assert_git_branch_remote_sync() {
    local -r remote=$1
    local -r branch=$2
    local -r local_commit="$(git rev-parse --verify -q "$branch^{commit}")"
    local -r remote_commit="$(git rev-parse --verify -q "$remote/$branch^{commit}")"

    if [ "$local_commit" != "$remote_commit" ]; then
        batslib_print_kv_single_or_multi 8 \
            'expected' "$local_commit" \
            'actual'   "$remote_commit" \
            | batslib_decorate 'Branch $branch is not in sync with $remote/$branch' \
            | fail
    fi
}

assert_git_branch_has_local() {
    local -r branch=$1

    if ! git rev-parse --verify -q "refs/heads/$branch" ; then
        echo "branch $branch does not exist" \
            | batslib_decorate 'ERROR: assert_git_branch_has_local' \
            | fail
    fi
}

assert_git_branch_can_fastforward() {
    local -r remote_branch=$1
    local -r local_branch=$2
    local rev
    local base

    rev=$(git rev-parse "$local_branch")
    base=$(git merge-base "$rev" "$remote_branch")

    if [ "$base" != "$rev" ]; then
        echo "branch $branch is not fast-forwardable" \
            | batslib_decorate 'ERROR: assert_git_branch_can_fastforward' \
            | fail
    fi
}
