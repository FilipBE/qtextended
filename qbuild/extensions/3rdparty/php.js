// This code comes from the PHP to JavaScript project.

/*!

\function basename
\ingroup functions_extension

\usage

\code
foo = basename("/foo/bar");
bar = basename("foo.txt", "txt");
###
foo=$$basename(/foo/bar)
bar=$$basename(foo.txt,txt)
###
\endcode

\description

Returns the basename of a file or removes the extension.

\sa {http://kevin.vanzonneveld.net/techblog/article/javascript_equivalent_for_phps_basename/}

*/
function basename(path, suffix) {
    // http://kevin.vanzonneveld.net
    // +   original by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
    // +   improved by: Ash Searle (http://hexmen.com/blog/)
    // +   improved by: Lincoln Ramsay
    // +   improved by: djmix
    // *     example 1: basename('/www/site/home.htm', '.htm');
    // *     returns 1: 'home'
 
    var b = path.replace(/^.*[\/\\]/g, '');
    if (typeof(suffix) == 'string' && b.substr(b.length-suffix.length) == suffix) {
        b = b.substr(0, b.length-suffix.length);
    }
    return b;
}

/*!

\function dirname
\ingroup functions_extension

\usage

\code
foo = dirname("/foo/bar");
###
foo=$$dirname(/foo/bar)
###
\endcode

\description

Returns the dirname of a file.

\sa {http://kevin.vanzonneveld.net/techblog/article/javascript_equivalent_for_phps_dirname/}

*/
function dirname(path) {
    // http://kevin.vanzonneveld.net
    // +   original by: Ozh
    // +   improved by: XoraX (http://www.xorax.info)
    // *     example 1: dirname('/etc/passwd');
    // *     returns 1: '/etc'
    // *     example 2: dirname('c:/Temp/x');
    // *     returns 2: 'c:/Temp'
    // *     example 3: dirname('/dir/test/');
    // *     returns 3: '/dir'
    
    return path.replace(/\\/g,'/').replace(/\/[^\/]*\/?$/, '');
}

