# NestorWeb

## What is this?

NestorWeb is a web server for MSX computers, with support for CGI scripts. As simple as that.

System requirements are:

* MSX computer running MSX-DOS 2 or [Nextor](https://github.com/Konamiman/Nextor).
* TCP/IP UNAPI implementation with support for passive TCP connections.

## Features and limitations

The good:

* Static content: serves files stored in the local filesystem.
  * `GET` and `HEAD` verbs supported.
  * Sends `Last-Modified` header as part of the response and honors the `If-Modified-Since` header if present in the request (provided that the file has a valid last modification date).
  * Sends files as attachments (with a `Content-Disposition: attachment` header) if `?a=1` is added to the request.
  * Requests containing `..` are rejected for security (so that files outside of the base directory aren't accessible)
* Directory listings: when a directory is requested is sends a list of the contained files .
  * Disabled by default, must be enabled in command line.
  * Sent with a `Cache-Control` header specifying a cache duration of 1 hour.
  * If a request for a directory doesn't end with `/` it sends a `308 Moved Permanently` response to the same location ending with `/`, this is necessary so that relative links to files within the directory render correctly in browsers.
* Default document: serves the `INDEX.HTM` file if a directory is requested (only if directory listings are disabled).
* Basic authentication support.
* Runs CGI scripts following [the CGI 1.1 specification](https://tools.ietf.org/html/rfc3875). See the [CGI development guide](CGI.md) for details.

The not-so-good:

* Serves only one client at the same time.
* No MIME types for static content, `Content-Type` is never sent when serving files.
* Absolutely no support for text encodings. I have no idea of what will happen when requesting a file with a non-ASCII name.
* Basic authentication support... but with one single set of credentials. CGI scripts can handle authentication by themselves, though.

## Usage

Run `NWEB.COM` like this:

    NWEB <base directory> [p=<port>] [v=0|1|2] [t=<timeout>] [d=0|1] [c=0|1] [a=0|1|2]

`<base directory>`: Static content will be served from this directory, for example a request for `<server IP>/FOO/BAR.TXT` will serve file `<base directory>\FOO\BAR.TXT`.

`<port>`: TCP port number where the server will be listening for requests, default is 80.

`t`: Inactivity timeout in seconds, if a client connects to the server but doesn't send any request in this time the connection will be terminated. Default value is 5 seconds.

`d`: Enable directory listings when 1 (disabled by default). When disabled, a request for a directory will try to serve the default document (`INDEX.HTM`) from that directory.

`c`: Enable support for CGI scripts when 1 (enabled by default). When disabled, CGI scripts will be served as regular static files.

`a`: Authentication mode:

* 0: No authentication required (default).
* 1: Authentication required for serving static content and directory listings.
* 2: Authentication required for serving static content and directory listings, and also for running CGI scripts.

In authentication modes 1 and 2 two environment items named `NWEB_USER` and `NWEB_PASSWORD` must exist, they specify the set of credentials that will be accepted.


## Environment items

NestorWeb makes use of the following environment items:

* `NWEB_TEMP`: Temporary directory to be used by NestorWeb when running CGI scripts (required to cache the request and the response). This is optional, if not present the value of the `TEMP` item will be used instead. If that one doesn't exist either, the directory where the `NWEB.COM` file is located will be used as the temporary directory.

* `NWEB_USER` and `NWEB_PASSWORD`: Credentials to be used for authentication. User and password supplied within the request must match these for the request to be successful if authentication is enabled.

* `NWEB_REALM`: Value of the "realm" part in the `WWW-Authenticate` header that will be sent to the client when authentication is required but no credentials were supplied in the request. This is optional, the default value is "NestorWeb".


## CGI scripts

A file will be recognized as a CGI script and executed if all of the following applies:

* Support for CGI scripts is enabled.
* The file has `.CGI` or `.COM` extension.
* The file is located in a directory named `CGI-BIN` relative to the base directory (exactly in that directory, NOT in a subdirectory).
* Authentication mode is 0 or 1, or it's 2 and proper credentials are supplied within the request.

CGI scripts are implemented as regular MSX-DOS executable programs that get the request via environment items and STDIN (file handle 0), and write the response to STDOUT (file handle 1). See the [CGI development guide](CGI.md) for details.

After a CGI script terminates its execution NestorWeb loads itself from disk again, therefore it's recommended to run `NWEB.COM` from a fast storage device, ideally the RAM disk. The same goes with the temporary directory: it's recommended to set `NWEB_TEMP` (or `TEMP`) to the RAM disk whenever possible (if a big enough RAM disk can be created to handle the biggest request and response that is expected to be handled by CGI scripts).


## Last but not least...

...if you like this project **[please consider donating!](http://www.konamiman.com/msx/msx-e.html#donate)** My kids need moar shoes!
