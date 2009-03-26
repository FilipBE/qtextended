#
#  Set these appropriately if a local mirror has been setup
#
# If the `dnsdomainname` is in $LOCAL_DOMAIN then any downloads will be fetched
# from the $LOCAL_DL mirror if they are required, instead of the above sites
LOCAL_DOMAIN=
LOCAL_DL=

# Display a message with some space around, if $VERBOSE is set
function msg()
{
    test -z $VERBOSE && return
    echo
    echo "$1"
}

DOM=`dnsdomainname | cut -f 1 -d '.'`
if [ $DOM -a $DOM=$LOCAL_DOMAIN ]; then 
{
    msg "Any sources needed will be fetched from local mirror $LOCAL_DL"
    BUSYBOX_DL=$LOCAL_DL
    LIDS_DL=$LOCAL_DL
    LINUX_DL=$LOCAL_DL
}
fi

export GPG_EXTNS='asc sign sig fail'

function gpgerror()
{
    base_file=`basename $1 .asc`
    echo Could not verify gpg signature $1
    echo Examine the output of the verify command to see why
    echo !!! Perhaps $base_file has been tampered with !!!
    die "Exiting due to security issue with downloaded file"
}

function gpg_check()
{
    gpg --keyserver pgpkeys.mit.edu --keyserver-options auto-key-retrieve --verify $1 || gpgerror $1
}

function download()
{
    file_name=`basename $1`
    wget $1 || die "Could not download $1"
    sig_ext_actual=asc
    for sig_ext in $GPG_EXTNS; do
    {
        if [ "$sig_ext" = "fail" ]; then
        {
            die "Could not download gpg signature for $1"
        }
        fi
        sig_ext_actual=$sig_ext
        wget $1.$sig_ext && break;
    }
    done
    gpg_check $file_name.$sig_ext_actual
}
