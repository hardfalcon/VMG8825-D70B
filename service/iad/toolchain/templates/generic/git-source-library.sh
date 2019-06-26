# this file is inteded to be sourced and used by bash scripts
# version 1.2.0

# Check prerequisites and set defaults
: ${GIT:=git}

SAS_GitSourceInfo()
{
    echo ">>> git-source: $@ <<<"
}

SAS_GitSourceError()
{
    echo >&2 ">>> git-source: ERROR: $@ <<<"
}

SAS_GitSourceExit()
{
    echo >&2 ">>> git-source: ERROR: $@ <<<"
    exit 1
}

SAS_GitSource_ChangeToGitDir()
{
    local git_dir=$1

    test -d $git_dir/.git

    test "$git_dir" != "$PWD" || return 0

    pushd $git_dir > /dev/null
}

SAS_GitSource_UpdateRemote()
{
    local git_remote=$1
    local git_fetch_url_new=$2
    local git_push_url_new=$3
    local git_fetch_url
    local git_push_url

    SAS_GitSourceInfo "Updating remote $git_remote"

    local git_fetch_url=$($GIT config --get remote.${git_remote}.url)
    if [ "$git_fetch_url" = "" ]; then
        SAS_GitSourceInfo "Adding new remote $git_remote with fetch URL $git_fetch_url_new"
        $GIT remote add $git_remote $git_fetch_url_new
    elif [ "$git_fetch_url" != "$git_fetch_url_new" ]; then
        SAS_GitSourceInfo "Updating fetch URL of $git_remote to $git_fetch_url_new"
        $GIT remote set-url $git_remote $git_fetch_url_new
    fi

    if [ ! -z $git_push_url_new ]; then
        local git_push_url=$($GIT config --get remote.${git_remote}.pushurl)
        if [ "$git_push_url" != "$git_push_url_new" ]; then
            SAS_GitSourceInfo "Updating push URL of $git_remote to $git_push_url_new"
            $GIT remote set-url --push $git_remote $git_push_url_new
        fi
    fi

    $GIT fetch -p $git_remote
}

SAS_GitSource_CheckTree()
{
    if ! $GIT update-index --ignore-submodules --refresh > /dev/null; then
        SAS_GitSourceError "you have unstaged changes"
        $GIT diff-files --name-status -r --ignore-submodules -- >&2
        return 1
    fi

    local diff=$($GIT diff-index --cached --name-status -r --ignore-submodules HEAD --)
    case "$diff" in
    ?*) SAS_GitSourceError "your index contains uncommitted changes"
        echo >&2 "$diff"
        return 1
        ;;
    esac

    return 0
}

SAS_GitSource_CheckOriginTags()
{
    local tags=($($GIT ls-remote --tags origin))
    local n=${#tags[*]}
    local i=0
    local j=0
    local it=`seq 0 2 $((n-1))`

    if [ $n -eq 0 ]; then
        return 0
    fi
    SAS_GitSourceInfo "Checking local tags"

    for i in $it; do
        j=$((i+1))
        local t=($($GIT rev-parse ${tags[j]##refs/tags/}))
        if [ $? -ne 0 ]; then
            SAS_GitSourceError "Tag ${tags[j]##refs/tags/} does not exist!"
        else
            if [ "${t[0]}" != "${tags[i]}" ]; then
                SAS_GitSourceError "Local tag \"${tags[j]##refs/tags/}\" is different to it's remote pendant!"
                SAS_GitSourceError "Please remove local tag \"${tags[j]##refs/tags/}\" first!"
                return 1
            else
                local k="${tags[j]##*^\{\}}"
                if [ "${#k}" -gt 0 ]; then
                    SAS_GitSourceInfo "Local tag \"${tags[j]##refs/tags/}\" equals remote origin"
                fi
            fi
        fi
    done
 
    return 0
}

# @brief: test if given name is a git branch or tag
# @param: name
# @return: 0 - if given name is a branch, != 0 otherwise
SAS_GitSource_HaveObject()
{
    local name=$1

    $GIT rev-parse $name^{object} > /dev/null 2>&1
    test $? -eq 0
}

# @brief: test if given name is a git branch
# @param: name
# @return: 0 - if given name is a branch, != 0 otherwise
SAS_GitSource_HaveBranch()
{
    local git_branch=$1

    if git rev-parse refs/heads/$git_branch > /dev/null 2>&1 ; then
        return 0
    fi
    if git rev-parse refs/remotes/$git_branch > /dev/null 2>&1 ; then
        return 0
    fi
    return 1
}

# @brief: test if given name is a local remote repository reference
# @param: name
# @return: 0 - if given name is a branch, != 0 otherwise
SAS_GitSource_HaveRemote()
{
    local git_remote=$1

    $GIT remote show $git_remote > /dev/null 2>&1
    test $? -eq 0
}

# @brief: test if given name is a git tag
# @param: name
# @return: 0 - if given name is a branch, != 0 otherwise
SAS_GitSource_HaveTag()
{
    local git_name=$1

    $GIT rev-parse $git_name^{tag} > /dev/null 2>&1
    test $? -eq 0
}

# @brief: test if given remote (url) contains a branch reference
# @param: remote url
# @param: branch name
# @return: 0 - if given remote contains the branch
SAS_GitSource_RemoteHaveBranch()
{
    local remote=$1
    local git_branch=$2

    $GIT ls-remote --exit-code --heads $remote refs/heads/$git_branch > /dev/null 2>&1
    test $? -eq 0
}

# @brief: test if given remote (url) contains a tag reference
# @param: remote url
# @param: tag name
# @return: 0 - if given remote contains the branch
SAS_GitSource_RemoteHaveTag()
{
    local remote=$1
    local git_branch=$2

    $GIT ls-remote --exit-code --tags $remote refs/tags/$git_branch > /dev/null 2>&1
    test $? -eq 0
}

SAS_GitSource_BranchCheckedOut()
{
    local current_branch=$(git symbolic-ref -q HEAD)
    current_branch=${current_branch##refs/heads/}
    current_branch=${current_branch:-HEAD}

    echo $current_branch
}

# @brief: return the sha1 value of a given git object (branch or tag)
# @param: object name
# @return: string with sha1 or ""
SAS_GitSource_ObjectHead()
{
    local git_object=$1

    if SAS_GitSource_HaveObject $git_object ; then
        $GIT rev-parse $git_object^{object}
    else
        echo ""
    fi
}

# @brief: return the sha1 value of a given tag name's first
#         commit, if the tag exists
# @param: tag name
# @return: string with sha1 or ""
SAS_GitSource_TagCommit()
{
    local git_tag=$1
    
    if SAS_GitSource_HaveTag $git_tag ; then
        $GIT rev-parse $git_tag^{commit}
    else
        echo ""
    fi
}

# @brief: return the sha1 value of a given tag name regardless of it's type
#         (annotation or commit), if the tag exists
# @param: tag name
# @return: string with sha1 or ""
SAS_GitSource_TagHead()
{
    local git_tag=$1
    
    if SAS_GitSource_HaveTag $git_tag ; then
        $GIT rev-parse $git_tag^{tag}
    else
        echo ""
    fi
}

# @brief: return the sha1 value of a given branch name
#         if the branch is a branch an exists
# @param: branch name
# @return: string with sha1 or ""
SAS_GitSource_BranchHead()
{
    local git_branch=$1
    
    if SAS_GitSource_HaveBranch $git_branch ; then
        $GIT rev-parse $git_branch^{object}
    else
        echo ""
    fi
}

# @brief: return the sha1 value of a given branch name of a remote repository
#         if the branch is a branch an exists
# @param: remote URL
# @param: branch name
# @return: string with sha1 or ""
SAS_GitSource_RemoteBranchHead()
{
    local remote=$1
    local git_branch=$2
    local git_rev=""

    SAS_GitSource_RemoteHaveBranch $remote $git_branch
    if [ $? -eq 0 ]; then
        git_rev=`SAS_GitSource_BranchHead refs/remotes/$remote/$git_branch`
    fi
    echo $git_rev
}

# @brief: return the sha1 value of a given tag name of a remote repository
#         if the tag exists
# @param: remote URL
# @param: tag name
# @return: string with sha1 or ""
SAS_GitSource_RemoteTagHead()
{
    local remote=$1
    local git_tag=$2
    local git_rev=""

    if SAS_GitSource_RemoteHaveTag $remote $git_tag ; then
        git_rev=`SAS_GitSource_ObjectHead $git_tag^{tag}`
    fi
    echo $git_rev
}

SAS_GitSource_CanFastForward()
{
    local git_remote_branch=$1
    local git_local_branch=$2

    local rev=$($GIT rev-parse $git_local_branch)
    local base=$($GIT merge-base $rev $git_remote_branch)

    test "$base" = "$rev"
}

SAS_GitSource_BranchesEqual()
{
    local git_rev1=$($GIT rev-parse $1)
    local git_rev2=$($GIT rev-parse $2)

    test "$git_rev1" = "$git_rev2"
}

SAS_GitSource_CommitCount()
{
    local cnt=$($GIT rev-list ^$2 $1 | wc -l)

    echo $cnt
}

SAS_GitSource_CreateBranchFrom()
{
    local git_local_branch=$1
    local git_remote_branch=$2

    if SAS_GitSource_HaveBranch $git_local_branch ; then
        $GIT branch -D $git_local_branch
    fi

    $GIT branch --no-track $git_local_branch $git_remote_branch
}

SAS_GitSource_CreateBranchFromTag()
{
    local git_branch=$1
    local git_tag=$2

    if ! SAS_GitSource_HaveBranch $git_branch ; then
        $GIT branch $git_branch $git_tag
    else
        SAS_GitSourceInfo "Branch $git_branch does exist in local repository - abort creation of branch"
    fi

}

SAS_GitSource_UpdateBranch()
{
    local git_remote=$1
    local git_branch=$2
    local git_branch_current

    git_branch_current=$(SAS_GitSource_BranchCheckedOut)

    SAS_GitSourceInfo "Updating branch $git_branch"

    if SAS_GitSource_HaveBranch $git_remote/$git_branch ; then
        if ! SAS_GitSource_HaveBranch $git_branch ; then
            $GIT branch --track $git_branch $git_remote/$git_branch
        else
            # Paranoia: always reconfigure branch tracking
            $GIT config "branch.$git_branch.remote" $git_remote
            $GIT config "branch.$git_branch.merge" "refs/heads/$git_branch"

            # Count commit difference between remote and local branch
            local l2r_cnt=$(SAS_GitSource_CommitCount $git_remote/$git_branch $git_branch)
            if [ $l2r_cnt -gt 0 ]; then
                if SAS_GitSource_CanFastForward $git_remote/$git_branch $git_branch; then
                    SAS_GitSourceInfo "Local branch $git_branch is behind $git_remote/$git_branch by $l2r_cnt commit(s), and can be fast-forwarded"

                    # If branches have not diverged, we can simply fast-forward merge the
                    # changes from remote branch
                    if [ "$git_branch_current" = "$git_branch" ]; then
                        SAS_GitSource_CheckTree || return 1
                        $GIT merge --ff $git_remote/$git_branch
                    else
                        $GIT fetch $git_remote $git_branch:$git_branch
                    fi

                    # else tree is also catched by if branch below
                fi
            fi

            # Count commit difference between local and remote branch
            local r2l_cnt=$(SAS_GitSource_CommitCount $git_branch $git_remote/$git_branch)
            if [ $r2l_cnt -gt 0 ]; then
                if SAS_GitSource_CanFastForward $git_branch $git_remote/$git_branch; then
                    SAS_GitSourceInfo "Local branch $git_branch is ahead of $git_remote/$git_branch by $r2l_cnt commit(s)"

                    # Do nothing
                    $GIT status -v
                else
                    SAS_GitSourceInfo "Branches $git_branch and $git_remote/$git_branch have diverged," \
                        "and have $r2l_cnt and $l2r_cnt different commit(s) each, respectively"

                    # if the currently checked out branch shall be updated, try a rebase onto
                    # the remote branch. Otherwise do nothing
                    if [ "$git_branch_current" = "$git_branch" ]; then
                        SAS_GitSource_CheckTree || return 1
                        SAS_GitSourceInfo "Trying to rebase branch $git_branch onto $git_remote/$git_branch"
                        $GIT rebase $git_remote/$git_branch || $GIT rebase --abort
                    fi
                fi
            fi

        fi
    else
        SAS_GitSourceInfo "Branch $git_branch does not exist in remote $git_remote"
    fi

    return 0
}


SAS_GitSource_UpdateMaster()
{
    local git_remote_name=$1

    SAS_GitSourceInfo "Updating master from $git_remote_name/master"

    $GIT checkout master
    $GIT merge $git_remote_name/master
}

SAS_GitSource_PushRemoteMaster()
{
    local git_remote_name="$1"

    SAS_GitSourceInfo "Pushing master to remote $git_remote_name"

    $GIT push $git_remote_name master
}

SAS_GitSource_CreatePatch()
{
    local index=$1
    local git_patch=$2
    local git_patch_dir=$3

    local patch_name=${git_patch//[\/^]/_}
    local patch_full=$(printf "%s/%04d-%s.patch" $git_patch_dir $index $patch_name)

    local patch_from=${git_patch%^*}
    local patch_to=${git_patch#*^}

    if ! SAS_GitSource_HaveBranch $patch_from ; then
        SAS_GitSourceInfo "Patch from branch $patch_from not found"
        return 1
    fi

    if ! SAS_GitSource_HaveBranch $patch_to ; then
        SAS_GitSourceInfo "Patch to branch $patch_to not found"
        return 1
    fi

    SAS_GitSourceInfo "Creating patch $patch_full"
    $GIT diff -p --no-renames --full-index --text --no-color $patch_from..$patch_to > $patch_full
}
