LANG=C
GIT='git'
GIT_IMPORT_ORIG=
GBP=$(which gbp)
test -z $GBP || GIT_IMPORT_ORIG="$GBP import-orig"
GIT_CHANGELOG=$SPH_TEMPLATES_PATH/gitlog-to-changelog

# Set error and exit handler
SAS_GitSource_ExitHandler()
{
    dirs -c
    exit
}
trap SAS_GitSource_ExitHandler EXIT
set -e

# Check prerequisites
: ${SAS_MOD_ROOT:?}
: ${SAS_GIT_ORIGIN_FETCH:?}
: ${SAS_GIT_ORIGIN_PUSH:?}
: ${SAS_GIT_CLONE_DIR:?}

if [ "$SAS_GITFLOW_ENABLE" = "y" ]; then
: ${SAS_GITFLOW_BRANCHES:?}
: ${SAS_GITFLOW_VERSIONS:?}
: ${SAS_GITFLOW_VERSION_MASTER:?}
: ${SAS_GITFLOW_VERSION_NEXT:?}
: ${SAS_GITFLOW_SPHAIRON_TAG:?}
: ${SAS_GITFLOW_SPHAIRON_MINOR_VERSION:=0}
: ${SAS_GITFLOW_PATCH_DIR:=patches/sphairon}
: ${SAS_GIT_DEF_BRANCH:=master}
fi

if [ "$SAS_GITSTASH_ENABLE" = "y" ]; then
: ${SAS_GIT_DEF_BRANCH:=master}
fi

: ${SAS_GIT_DEF_BRANCH:?}

test -d $SAS_MOD_ROOT

if [ -f $SPH_TEMPLATES_PATH/git-source-library.sh ] ; then
    source $SPH_TEMPLATES_PATH/git-source-library.sh
else
    echo "Unable to source $SPH_TEMPLATES_PATH/git-source-library.sh"
    exit 1
fi

SAS_GitSource_GitClone()
{
    SAS_GitSourceInfo "Cloning new git tree from $SAS_GIT_ORIGIN_FETCH"
    mkdir -p $SAS_MOD_ROOT
    pushd $SAS_MOD_ROOT > /dev/null
    $GIT clone $SAS_GIT_ORIGIN_FETCH $SAS_GIT_CLONE_DIR

    if [ ! -z $SAS_GIT_ORIGIN_PUSH ]; then
        SAS_GitSourceInfo "Updating origin's push URL with $SAS_GIT_ORIGIN_PUSH"
        pushd $SAS_GIT_CLONE_DIR > /dev/null
        $GIT remote set-url --push origin $SAS_GIT_ORIGIN_PUSH
        popd > /dev/null
    fi

    SAS_GitSource_AddLocalReference
    SAS_GitSource_ShowAlternates

    popd > /dev/null
}

SAS_Git_GitCloneShallow()
{
    SAS_GitSourceInfo "Shallow-cloning new git tree from $SAS_GIT_ORIGIN_FETCH"
    mkdir -p $SAS_MOD_ROOT
    pushd $SAS_MOD_ROOT > /dev/null

    rm -rf $SAS_GIT_CLONE_DIR
    $GIT clone --depth=1 --no-single-branch $SAS_GIT_ORIGIN_FETCH $SAS_GIT_CLONE_DIR

    popd > /dev/null
}

SAS_GitSource_UpdateBranches()
{
    local git_remote=$1
    local git_branch

    if [ "$git_remote" = "origin" ]; then
        SAS_GitSource_UpdateBranch origin master
    fi

    for git_branch in ${SAS_GIT_BRANCHES[*]}; do
        SAS_GitSource_UpdateBranch $git_remote $git_branch
    done
}

SAS_GitFlow_UpdateBranches()
{
    local git_remote=$1
    local git_branch
    local gitflow_branches=(pristine-tar upstream next master)

    for git_branch in ${SAS_GITFLOW_BRANCHES[*]} ${gitflow_branches[*]}; do
        SAS_GitSource_UpdateBranch $git_remote $git_branch
    done

    for git_branch in ${SAS_GITFLOW_SUPPORT[*]}; do
        SAS_GitSource_UpdateBranch $git_remote support/$git_branch
    done

    SAS_GitFlow_CreateMaintenanceBranches $git_remote

}

SAS_GitStash_UpdateBranches()
{
    local git_remote=$1
    local git_branch
    local gitflow_branches=(pristine-tar upstream next master)

    for git_branch in ${SAS_GITFLOW_BRANCHES[*]} ${gitflow_branches[*]}; do
        SAS_GitSource_UpdateBranch $git_remote $git_branch
    done

    for git_branch in ${SAS_GITFLOW_SUPPORT[*]}; do
        SAS_GitSource_UpdateBranch $git_remote support/$git_branch
    done
}

SAS_GitFlow_CreateMaintenanceBranches()
{
    local git_remote=$1
    local force_creation=$2
    local git_branch
    local do_create=

    if [ "$SAS_GITFLOW_MAINTENANCE_CREATE_BRANCH" = "yes" ]; then
        do_create=yes
    fi
    if [ "$force_creation" = "force" ]; then
        do_create=yes
    fi

    for git_branch in ${SAS_GITFLOW_MAINTENANCE_VERSIONS[*]}; do
        SAS_GitSource_UpdateBranch $git_remote master-$git_branch
        if ! SAS_GitSource_HaveBranch master-$git_branch ; then
            if [ "$do_create" = "yes" ]; then
                SAS_GitSource_CreateBranchFromTag master-$git_branch $git_branch
            else
                SAS_GitSourceInfo "Creation of maintenance branch master-$git_branch disabled!"
            fi
        fi

        SAS_GitSource_UpdateBranch $git_remote next-$git_branch
        if ! SAS_GitSource_HaveBranch next-$git_branch ; then
            if [ "$do_create" = "yes" ]; then
                SAS_GitSource_CreateBranchFromTag next-$git_branch $git_branch
            else
                SAS_GitSourceInfo "Creation of maintenance branch next-$git_branch disabled!"
            fi
        fi
    done
}

SAS_GitSource_CheckoutDefaultBranch()
{
    SAS_GitSourceInfo "Checking out default branch $SAS_GIT_DEF_BRANCH"

    $GIT checkout $SAS_GIT_DEF_BRANCH
}

SAS_GitSource_CreatePatches()
{
    local git_patch_dir=$1
    local index=1

    if [ -d $git_patch_dir ]; then
        rm -rf $git_patch_dir/*
    else
        mkdir -p $git_patch_dir
    fi

    for git_patch in ${SAS_GIT_PATCH_RANGES[*]}; do
        SAS_GitSource_CreatePatch $index $git_patch $git_patch_dir
        index=$((index + 1))
    done
}

SAS_GitSource_CreatePatchSingle()
{
    local patch_path=$1
    local patch_index=$2
    local git_rev_from=$3
    local git_rev_to=$4

    if ! SAS_GitSource_HaveObject $git_rev_from ; then
        SAS_GitSourceError "Cannot find git revision for '$git_rev_from'"
        return 1
    fi

    if ! SAS_GitSource_HaveObject $git_rev_to ; then
        SAS_GitSourceError "Cannot find git revision for '$git_rev_to'"
        return 1
    fi

    # convert all / ^ . - to _
    local patch_from=${git_rev_from//[\/^.-]/_}
    local patch_to=${git_rev_to//[\/^.-]/_}
    local patch_name=$(printf "%04d-%s__%s.patch" $patch_index $patch_from $patch_to)

    SAS_GitSourceInfo "Creating patch '$patch_name'"

    $GIT diff -p --no-renames --full-index --text --no-color $git_rev_from..$git_rev_to > $patch_path/$patch_name
}

SAS_GitSource_CreatePatchsetForTagVsUpstream()
{
    local patch_root=$1
    local patch_version=$2
    local patch_prefix=$3
    local last_tag=$4
    local patch_path=$patch_root/$patch_prefix

    if [ -d $patch_path ]; then
        rm -rf $patch_path/*
    else
        mkdir -p $patch_path
    fi

    # need special sorting if versions jump from sphaironN to sphaironNN
    # or from sphaironN.M to sphaironN.MM
    local git_tags=()
    for i in $(seq 1 1 3); do
        git_tags+=($($GIT tag -l | awk -e "/^$patch_version$SAS_GITFLOW_SPHAIRON_TAG[[:digit:]]{$i}\$/ { print \$1 }"))
        for j in $(seq 1 1 3); do
            git_tags+=($($GIT tag -l | awk -e "/^$patch_version$SAS_GITFLOW_SPHAIRON_TAG[[:digit:]]{$i}.[[:digit:]]{$j}\$/ { print \$1 }"))
        done
    done

    local patch_from=upstream/$patch_version
    local patch_to=
    local patch_index=1

    for git_tag in ${git_tags[*]}; do
        patch_to=$git_tag
        SAS_GitSource_CreatePatchSingle $patch_path $patch_index $patch_from $patch_to
        [ "$git_tag" != "$last_tag" ] || break
        patch_from=$git_tag
        patch_index=$((patch_index + 1))
    done
}

SAS_GitSource_CreatePatchsetForTagsOnBranch()
{
    local patch_root=$1
    local patch_version=$2
    local patch_prefix=$3
    local last_tag=$4
    local start_index=$5
    local patch_path=$patch_root

    if [ ! -d $patch_path ]; then
        mkdir -p $patch_path
    fi

    # retrieve all tags starting with $patch_version prefix
    local git_tags=()
    git_tags+=($($GIT tag -l $patch_version.*))

    local patch_from=$patch_version
    local patch_to=
    local patch_index=1
    if [ ! -z $start_index ] ; then
        patch_index=$start_index
    fi
    for git_tag in ${git_tags[*]}; do
        patch_to=$git_tag
        SAS_GitSource_CreatePatchSingle $patch_path $patch_index $patch_from $patch_to
        [ "$git_tag" != "$last_tag" ] || break
        patch_from=$git_tag
        patch_index=$((patch_index + 1))
    done
}

SAS_GitSource_CreatePatchsetForSupport()
{
    local patch_root=$1
    local patch_version=$2
    local patch_prefix=$3
    local patch_path=$patch_root/$patch_prefix

    local upstream_version=${patch_version%$SAS_GITFLOW_SPHAIRON_TAG*}
    local last_tag=$($GIT tag --contains support/$patch_prefix)
    SAS_GitSource_CreatePatchsetForTagVsUpstream $patch_root $upstream_version $patch_prefix $last_tag
}

SAS_GitSource_CreatePatchForBranchVsUpstream()
{
    local patch_root=$1
    local patch_version=$2
    local patch_branch=$3

    local patch_path=$patch_root

    if [ -d $patch_path ]; then
        rm -rf $patch_path/*
    else
        mkdir -p $patch_path
    fi

    local patch_from=upstream/$patch_version
    local patch_to=$patch_branch
    local patch_index=1

    SAS_GitSource_CreatePatchSingle $patch_path $patch_index $patch_from $patch_to
}

SAS_GitSource_CreatePatchesGitFlow()
{
    local patch_root=$1
    local maintenance_root_branch=

    for patch_version in ${SAS_GITFLOW_VERSIONS[*]}; do
        SAS_GitSourceInfo "Creating patches for upstream version '$patch_version'"
        SAS_GitSource_CreatePatchsetForTagVsUpstream $patch_root $patch_version $patch_version
    done

    for patch_version in ${SAS_GITFLOW_SUPPORT[*]}; do
        SAS_GitSourceInfo "Creating patches for support version '$patch_version'"
        SAS_GitSource_CreatePatchsetForSupport $patch_root $patch_version $patch_version
    done

    SAS_GitSourceInfo "Creating patches for branch 'master' based on upstream version '$SAS_GITFLOW_VERSION_MASTER'"
    SAS_GitSource_CreatePatchsetForTagVsUpstream $patch_root $SAS_GITFLOW_VERSION_MASTER master

    SAS_GitSourceInfo "Creating patches for branch 'next' based on upstream version '$SAS_GITFLOW_VERSION_NEXT'"
    SAS_GitSource_CreatePatchForBranchVsUpstream $patch_root/next $SAS_GITFLOW_VERSION_NEXT next

    for patch_version in ${SAS_GITFLOW_MAINTENANCE_VERSIONS[*]}; do
        # strip of SAS_GITFLOW_SPHAIRON_TAG
        maintenance_root_branch=${patch_version%%$SAS_GITFLOW_SPHAIRON_TAG*}

        SAS_GitSourceInfo "Creating patches for branch 'master-$patch_version'"
        SAS_GitSource_CreatePatchForBranchVsUpstream $patch_root/master-$patch_version $maintenance_root_branch $patch_version
        SAS_GitSource_CreatePatchsetForTagsOnBranch $patch_root/master-$patch_version $patch_version master-$patch_version "" 2

        SAS_GitSourceInfo "Creating patches for branch 'next-$patch_version'"
        SAS_GitSource_CreatePatchForBranchVsUpstream $patch_root/next-$patch_version $maintenance_root_branch next-$patch_version
    done
}

SAS_GitSource_CreateChangelogForVersion()
{
    local git_patch=$1
    local changelog_dir=$2
    local changelog_file=$changelog_dir/CHANGELOG
    local changelog_tmp=$changelog_dir/CHANGELOG.tmp

    local patch_from=${git_patch%^*}
    local patch_to=${git_patch#*^}

    if ! SAS_GitSource_HaveObject $patch_from ; then
        SAS_GitSourceInfo "Patch from branch $patch_from not found"
        return 1
    fi

    if ! SAS_GitSource_HaveObject $patch_to ; then
        SAS_GitSourceInfo "Patch to branch $patch_to not found"
        return 1
    fi

    mv $changelog_file $changelog_tmp

    SAS_GitSourceInfo "Creating changelog for version $patch_to"

    echo -e "$patch_to:\n" > $changelog_file
    $GIT_CHANGELOG --format=%s $patch_from..$patch_to -- --no-merges >> $changelog_file

    echo -e "\n" >> $changelog_file
    cat $changelog_tmp >> $changelog_file
    rm $changelog_tmp
}

SAS_GitSource_CreateChangelog()
{
    local changelog_dir=$1

    if [ ! -d $changelog_dir ]; then
        mkdir -p $changelog_dir
    fi

    echo -n > $changelog_dir/CHANGELOG

    for git_patch in ${SAS_GIT_PATCH_RANGES[*]}; do
        SAS_GitSource_CreateChangelogForVersion $git_patch $changelog_dir
    done
}

SAS_GitSource_UseGit()
{
    [ "$SKIP_GIT" = "y" ] && return 1

    [ -e $SAS_MOD_ROOT/use_git ] && return 0

    [ "$USE_GIT" = "y" ] && return 0

    return 1
}

SAS_GitSource_UseGitFlow()
{
    [ "$SAS_GIT_USE_GITFLOW" = "y" ] && return 0
    [ "$SAS_GITFLOW_ENABLE" = "y" ] && return 0

    return 1
}

SAS_GitSource_UseGitStash()
{
    [ "$SAS_GIT_USE_STASH" = "y" ] && return 0
    [ "$SAS_GITSTASH_ENABLE" = "y" ] && return 0

    return 1
}

SAS_GitFlow_InitConfig()
{
    local branch_suffix=
    SAS_GitSourceInfo "Preparing tree for git-flow"

    if [ ! -z $SAS_GITFLOW_MAINTENANCE_VERSION ] ; then
        branch_suffix="-$SAS_GITFLOW_MAINTENANCE_VERSION"
    fi
    SAS_GitSourceInfo "git-flow 'master' branch is set to master$branch_suffix"
    SAS_GitSourceInfo "git-flow 'development' branch is set to next$branch_suffix"
    git config gitflow.branch.master master$branch_suffix
    git config gitflow.branch.develop next$branch_suffix

    git config gitflow.prefix.feature feature/
    git config gitflow.prefix.release release/
    git config gitflow.prefix.hotfix hotfix/
    git config gitflow.prefix.support support/
    git config gitflow.prefix.versiontag ""
}

SAS_GitSource_UpdateGitTree()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR

    SAS_GitSource_UseGit || return 0

    if [ ! -d $git_clone_dir ]; then
        SAS_GitSource_GitClone
        SAS_GitSource_ChangeToGitDir $git_clone_dir
    elif [ ! -d $git_clone_dir/.git ]; then
        rm -rf $git_clone_dir
        SAS_GitSource_GitClone
        SAS_GitSource_ChangeToGitDir $git_clone_dir
    else
        SAS_GitSource_ChangeToGitDir $git_clone_dir

        SAS_GitSource_AddLocalReference
        SAS_GitSource_ShowAlternates

        SAS_GitSource_UpdateRemote origin $SAS_GIT_ORIGIN_FETCH $SAS_GIT_ORIGIN_PUSH
    fi

    SAS_GitSource_UpdateBranches origin
}

SAS_GitFlow_UpdateTree()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR

    SAS_GitSource_UseGit || return 0

    if [ ! -d $git_clone_dir ]; then
        SAS_GitSource_GitClone
        SAS_GitSource_ChangeToGitDir $git_clone_dir
    elif [ ! -d $git_clone_dir/.git ]; then
        rm -rf $git_clone_dir
        SAS_GitSource_GitClone
        SAS_GitSource_ChangeToGitDir $git_clone_dir
    else
        SAS_GitSource_ChangeToGitDir $git_clone_dir
        git_current_branch=$(SAS_GitSource_BranchCheckedOut)

        SAS_GitSource_AddLocalReference
        SAS_GitSource_ShowAlternates

        SAS_GitSource_UpdateRemote origin $SAS_GIT_ORIGIN_FETCH $SAS_GIT_ORIGIN_PUSH
    fi

    SAS_GitFlow_InitConfig
    SAS_GitFlow_UpdateBranches origin
    SAS_GitSource_CheckOriginTags
}

SAS_GitStash_UpdateTree()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR

    if [ ! -d $git_clone_dir ]; then
        SAS_GitSource_GitClone
        SAS_GitSource_ChangeToGitDir $git_clone_dir
    elif [ ! -d $git_clone_dir/.git ]; then
        rm -rf $git_clone_dir
        SAS_GitSource_GitClone
        SAS_GitSource_ChangeToGitDir $git_clone_dir
    else
        SAS_GitSource_ChangeToGitDir $git_clone_dir

        SAS_GitSource_AddLocalReference
        SAS_GitSource_ShowAlternates

        SAS_GitSource_UpdateRemote origin $SAS_GIT_ORIGIN_FETCH $SAS_GIT_ORIGIN_PUSH
    fi

    SAS_GitStash_UpdateBranches origin
    SAS_GitSource_CheckOriginTags
}

SAS_GitSource_UpdateGitPatches()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local git_patch_dir=$SAS_MOD_ROOT/$SAS_GIT_PATCH_DIR

    # check prerequisites
    : ${SAS_GIT_PATCH_DIR:?}
    : ${SAS_GIT_PATCH_RANGES:?}

    SAS_GitSource_ChangeToGitDir $git_clone_dir
    SAS_GitSource_CheckTree
    SAS_GitSource_CreatePatches $git_patch_dir
}

SAS_GitFlow_UpdatePatches()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local git_patch_root=$SAS_MOD_ROOT/$SAS_GITFLOW_PATCH_DIR

    SAS_GitSource_ChangeToGitDir $git_clone_dir
    SAS_GitSource_CheckTree
    SAS_GitSource_CreatePatchesGitFlow $git_patch_root
}

SAS_GitSource_UpdateChangelog()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local changelog_dir=$SAS_MOD_ROOT/$SAS_CHANGELOG_DIR

    # check prerequisites
    : ${GIT_CHANGELOG:?}
    : ${SAS_CHANGELOG_DIR:?}
    : ${SAS_GIT_PATCH_RANGES:?}

    SAS_GitSource_ChangeToGitDir $git_clone_dir
    SAS_GitSource_CreateChangelog $changelog_dir
}

SAS_GitSource_AutoRebase()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR

    # check prerequisites
    : ${SAS_GIT_AUTO_REBASE_BRANCHES:?}

    SAS_GitSource_ChangeToGitDir $git_clone_dir
    SAS_GitSource_CheckTree

    for i in $(seq 0 $((${#SAS_GIT_AUTO_REBASE_BRANCHES[@]} - 1))); do
        local git_branch=${SAS_GIT_AUTO_REBASE_BRANCHES[$i]}
        local branch_base=${git_branch%^*}
        local branch_cur=${git_branch#*^}
        SAS_GitSourceInfo "Auto-rebasing branch $branch_cur on $branch_base"
        $GIT checkout $branch_cur
        $GIT rebase -v $branch_base
    done
}

SAS_GitSource_ShowAlternates()
{
    local alternates=

    SAS_GitSource_ChangeToGitDir $git_clone_dir

    if [ -f  ./.git/objects/info/alternates ] ; then
        alternates=$(cat ./.git/objects/info/alternates)
        SAS_GitSourceInfo "CAUTION!"
        SAS_GitSourceInfo "$alternates is used as a local git reference repository."
        SAS_GitSourceInfo "Do not remove it until you really know what you do!"
        SAS_GitSourceInfo "Before removing, call the update handler with GIT_LOCAL_REFERENCE_REMOVE=y"
    fi
}

# function to make use of a local reference in a module
SAS_GitSource_AddLocalReference()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local git_reference_objects=$GIT_LOCAL_REFERENCE_DIR/$SAS_GIT_LOCAL_REFERENCE_NAME/.git/objects

    if [ x"$GIT_LOCAL_REFERENCE_REMOVE" = "xy" ] ; then
        SAS_GitSource_RemoveAllLocalReferences
        return 0
    fi

    if [ -z $GIT_LOCAL_REFERENCE_DIR ] ; then
        return 0
    fi

    # check prerequisites
    : {SAS_GIT_LOCAL_REFERENCE_NAME:?}

    if [ -d "$git_reference_objects" ] ; then
        SAS_GitSource_ChangeToGitDir $git_clone_dir

        if [ ! -f  ./.git/objects/info/alternates ] ; then
            echo "$git_reference_objects" > ./.git/objects/info/alternates
        fi
    else
        if [ -z $SAS_GIT_LOCAL_REFERENCE_NAME ]; then
                return 0
        fi

        # however tell the user what's possible
        SAS_GitSourceInfo "NOTE:"
        SAS_GitSourceInfo " If you want to use local git reference for module $SAS_MOD_ROOT"
        SAS_GitSourceInfo " you have to create $GIT_LOCAL_REFERENCE_DIR/$SAS_GIT_LOCAL_REFERENCE_NAME with command"
        SAS_GitSourceInfo "   > git clone $SAS_GIT_LOCAL_REFERENCE_TREE $GIT_LOCAL_REFERENCE_DIR/$SAS_GIT_LOCAL_REFERENCE_NAME"
        SAS_GitSourceInfo " before calling update-hook, typically done by ./meta/update-hook.sh"
    fi
}

SAS_GitSource_RemoveAllLocalReferences()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR

    # check prerequisites

    SAS_GitSource_ChangeToGitDir $git_clone_dir

    if [ -f ./.git/objects/info/alternates ] ; then
        SAS_GitSourceInfo "Repacking git repository and copy from local references"
        git repack -a
        SAS_GitSourceInfo "remove all local references"
        rm ./.git/objects/info/alternates
    fi
}

SAS_GitSource_ImportTarball()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local tarball=$(readlink -f $1)
    local upstream_tag=$2
    local upstream_branch=upstream
    local vcs_tag_arg=
    local interactive_mode=--no-interactive

    : ${GIT_IMPORT_ORIG:? - try apt-get install git-buildpackage}

    test -f $tarball || error "cannot find tarball $tarball"

    SAS_GitSource_ChangeToGitDir $git_clone_dir
    SAS_GitSource_CheckTree

    if ! SAS_GitSource_HaveBranch $upstream_branch ; then
        $GIT checkout --orphan $upstream_branch
    fi

    if [ "$upstream_tag" != "" ]; then
        vcs_tag_arg="--upstream-vcs-tag $upstream_tag"
    fi
 
    echo "start importing from tarball $1"
    echo "If tarball is already in the form package-name_version.orig.tar.gz,"
    echo "the version information is read from the tarball's filename. "
    echo "Providing a valid tarballname can be a struggling task."
    echo "Working examples are:"
    echo "            libabc-x.y.z.tar.gz"
    echo "            libabc-lib_x.y.z.orig.tar.gz"
    echo "            libabc-lib-x.y.z.orig.tar.gz"
    echo "            libabc_y.x.z.orig.tar.gz"
    echo ""
    echo "Non-working examples:"
    echo "            libabc_x.y.z.tar.gz"
    echo "            libabc-lib_x.y.z.tar.gz"
    echo "            libabc-lib-x.y.z.tar.gz"
    echo ""
    echo "If you have problems on automatic import, try setting"
    echo "\"GIT_IMPORT_INTERACTIVE=yes\" before using this method."


    if [ "$GIT_IMPORT_INTERACTIVE" = "yes" ]; then
        interactive_mode=--interactive
        echo "Using interactive mode for $GIT_IMPORT_ORIG"
    fi

    $GIT_IMPORT_ORIG -v $interactive_mode  \
        --debian-branch=next --pristine-tar $vcs_tag_arg $tarball
}

SAS_GitSource_CreateEmptyRepo()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local project_dir="$1"

    if [ -z ${project_dir} ]; then
        project_dir=${git_clone_dir}
    fi

    if [ "${project_dir}" = "${SAS_MOD_ROOT}" ] ; then
        SAS_GitSourceExit "Cannot use ${SAS_MOD_ROOT}"
    fi

    if [ ! -d ${project_dir} ] ; then
        mkdir -p ${project_dir}
    fi
    
    pushd $project_dir > /dev/null

    if [ -d .git ] ; then
        # check, if it is a empty dir
        if SAS_GitSource_HaveBranch master ; then
            SAS_GitSourceError "cannot use repository in $project_dir"
            SAS_GitSourceExit "it already contains a non-empty git repo with a branch \"master\""
        fi
    else
        SAS_GitSourceInfo "Initialize empty git directory in $(pwd)"
        $GIT init .
    fi

    if ! SAS_GitSource_HaveRemote origin ; then
        if [ ! -z $SAS_GIT_ORIGIN_FETCH ]; then
            SAS_GitSourceInfo "Adding origin with URL $SAS_GIT_ORIGIN_FETCH"
            $GIT remote add origin $SAS_GIT_ORIGIN_FETCH
        fi

        if [ ! -z $SAS_GIT_ORIGIN_PUSH ]; then
            SAS_GitSourceInfo "Updating origin with push URL $SAS_GIT_ORIGIN_PUSH"
            $GIT remote set-url --push origin $SAS_GIT_ORIGIN_PUSH
        fi
    fi

    if ! SAS_GitSource_HaveBranch master ; then
        SAS_GitSourceInfo "Create initial \"Root commit\" on empty master"
        $GIT commit --allow-empty -m "Root commit"
    fi
    if ! SAS_GitSource_HaveBranch next ; then
        SAS_GitSourceInfo "Create branch next based on master"
        $GIT checkout -b next master
    fi
    if ! SAS_GitSource_HaveBranch upstream ; then
        SAS_GitSourceInfo "Create branch upstream based on master"
        $GIT checkout -b upstream master
    fi
    $GIT checkout next

    SAS_GitFlow_InitConfig

    SAS_GitSourceInfo "--- Finished module setup ---"
    SAS_GitSourceInfo "Created initial branch structure in git module ${project_dir}"
    SAS_GitSourceInfo "You may want to change your push origin url via"
    SAS_GitSourceInfo "  # git remote set-url --push origin <url>"
    SAS_GitSourceInfo "If you made your changes, just push all the branches to remote origin repo via"
    SAS_GitSourceInfo "  # git push origin master"
    SAS_GitSourceInfo "  # git push origin next"
    SAS_GitSourceInfo "  # git push origin upstream"
    SAS_GitSourceInfo "Do not forget to push git flow branches with prefixes"
    SAS_GitSourceInfo " feature/* release/* hotfix/* and support/*"
}

SAS_GitStash_CreateEmptyRepo()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local project_dir="$1"

    if [ -z ${project_dir} ]; then
        project_dir=${git_clone_dir}
    fi

    if [ "${project_dir}" = "${SAS_MOD_ROOT}" ] ; then
        SAS_GitSourceExit "Cannot use ${SAS_MOD_ROOT}"
    fi

    if [ ! -d ${project_dir} ] ; then
        mkdir -p ${project_dir}
    fi
    
    pushd $project_dir > /dev/null

    if [ -d .git ] ; then
        # check, if it is a empty dir
        if SAS_GitSource_HaveBranch master ; then
            SAS_GitSourceError "cannot use repository in $project_dir"
            SAS_GitSourceExit "it already contains a non-empty git repo with a branch \"master\""
        fi
    else
        SAS_GitSourceInfo "Initialize empty git directory in $(pwd)"
        $GIT init .
    fi

    if ! SAS_GitSource_HaveRemote origin ; then
        if [ ! -z $SAS_GIT_ORIGIN_FETCH ]; then
            SAS_GitSourceInfo "Adding origin with URL $SAS_GIT_ORIGIN_FETCH"
            $GIT remote add origin $SAS_GIT_ORIGIN_FETCH
        fi

        if [ ! -z $SAS_GIT_ORIGIN_PUSH ]; then
            SAS_GitSourceInfo "Updating origin with push URL $SAS_GIT_ORIGIN_PUSH"
            $GIT remote set-url --push origin $SAS_GIT_ORIGIN_PUSH
        fi
    fi

    if ! SAS_GitSource_HaveBranch master ; then
        SAS_GitSourceInfo "Create initial \"Root commit\" on empty master"
        $GIT commit --allow-empty -m "Root commit"
    fi
    if ! SAS_GitSource_HaveBranch next ; then
        SAS_GitSourceInfo "Create branch next based on master"
        $GIT checkout -b next master
    fi
    if ! SAS_GitSource_HaveBranch upstream ; then
        SAS_GitSourceInfo "Create branch upstream based on master"
        $GIT checkout -b upstream master
    fi

    $GIT checkout next

    SAS_GitSourceInfo "--- Finished module setup ---"
    SAS_GitSourceInfo "Created initial branch structure in git module ${project_dir}"
    SAS_GitSourceInfo "You may want to change your push origin url via"
    SAS_GitSourceInfo "  # git remote set-url --push origin <url>"
    SAS_GitSourceInfo "If you made your changes, just push all the branches to remote origin repo via"
    SAS_GitSourceInfo "  # git push origin master"
    SAS_GitSourceInfo "  # git push origin next"
    SAS_GitSourceInfo "  # git push origin upstream"
}


SAS_Git_UpdateHook()
{
    if SAS_GitSource_UseGitFlow ; then
        SAS_GitFlow_UpdateTree
    elif SAS_GitSource_UseGitStash ; then
        SAS_GitStash_UpdateTree
    else
        SAS_GitSource_UpdateGitTree
    fi
}

SAS_Git_ExportHook()
{
    if ! SAS_GitSource_UseGitStash ; then
        SAS_GitSource_UseGit || return 0
    fi

    SAS_Git_GitCloneShallow
}

SAS_Git_UseGit()
{
    SAS_GitSource_UseGit && return 0

    touch $SAS_MOD_ROOT/use_git
}

SAS_GitSource_ApplyGitPatches()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local git_current_branch=$SAS_GIT_DEF_BRANCH
    local patches_dir="$1"
    local patches
    local patch_dir

    SAS_GitSource_UseGit || return 0

    if [ -f "$patches_dir" ] ; then
        patch_dir=$(dirname $patches_dir)
        pushd ${patch_dir}
        patch_dir=$(pwd)
        popd
        patches=$patch_dir/$(basename $patches_dir)
    elif [ -d "$patches_dir" ] ; then
        pushd ${patches_dir}
        patches_dir=$(pwd)
        popd
        echo "Collecting patches in $patches_dir"
        patches=$(find ${patches_dir}/*.patch)
    else
        echo "Directory containing patches does not exist"
        return 1
    fi

    if [ ! -d ${git_clone_dir}/.git ]; then
        SAS_Git_UpdateHook
        status=$?
    fi

    if [ ! -d ${git_clone_dir}/.git ]; then
        echo "Missing git repository $git_clone_dir"
        return 1
    fi

    if [ "${GITIGNORE_FILE}" != "" ] ; then
        echo "Using ${GITIGNORE_FILE} as .gitignore"
        cp -f ${GITIGNORE_FILE} ${git_clone_dir}/.gitignore
    fi

    echo ">>> Patches to apply <<<"
    for p in ${patches}; do echo " $p " ; done
    echo ""

    echo ">>> Applying patches <<<"
    pushd ${git_clone_dir}
    for p in ${patches}; do
        git am --reject --keep-cr $p || exit 1;
    done
    popd
}

SAS_GitStash_FindSasConfigGitBranch()
{
    local git_clone_dir=$SAS_MOD_ROOT/$SAS_GIT_CLONE_DIR
    local sas_config="$1"
    local comps
    local git_branch
    local git_rev
    local start
    local config
    
    start="${sas_config//[^-]}"
    start=${#start}

    # put all sas_config components into comps as list
    # so 3.10-generic-debug-master
    # gets: "3.10 generic debug master"
    comps=(${sas_config//-/ })

    pushd $git_clone_dir > /dev/null 2>&1
    for a in $(seq $start -1 0); do
        git_branch=${comps[@]:$a}
        git_branch=${git_branch// /-}
        echo "checking for branch $git_branch" >/dev/null

        if SAS_GitSource_HaveObject $git_branch ; then
            echo "Found matching local tag/branch/hash: $git_rev" > /dev/null
            git_rev=$git_branch
            break
        fi
        if [ "$SAS_GITSTASH_TEMPLATE_OFFLINE_MODE" = y ] ; then
            continue
        fi
        if SAS_GitSource_RemoteHaveBranch origin $git_branch ; then
            echo "Found remote origin branch: $git_rev" > /dev/null
            git_rev=$git_branch
            break
        fi
        if SAS_GitSource_RemoteHaveTag origin $git_branch ; then
            echo "Found remote origin tag: $git_rev" > /dev/null
            git_rev=$git_branch
            break
        fi
    done
    popd > /dev/null 2>&1
    if [ ${#git_rev} -gt 1 ] ; then
        echo "$git_rev"
    fi
}
