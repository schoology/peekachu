peekachu
========

peekachu is a utility for peeking at key values in memcache. It support compression via the [FastLZ](http://fastlz.org/) library.

The peekachu utility was created in order to help debug data issues when using more complex memcache client libraries such a PHP's memcached.
Libraries like PHP memcached automatically serialize and (optionally) compress values and so it can become difficult to access a key's raw value.

Example Usage
==============

To get help/usage information:

```
$ peekachu --help
Usage: peekachu [OPTION...] KEY
peekachu -- Allows users to peek at compressed/raw values in memcache.

  -f, --file=FILE            File to get key's value from. Causes key to be
                             set. Use - to read from stdin, otherwise path to
                             FILE.
  -h, --host=HOST            Hostname of the memcache server to connect to.
  -p, --port=PORT            Port number of the memcache server to connect to.
  -r, --raw                  Forces peekachu to always write the raw key value
                             to stdout.
  -z, --compression          Enabled compression/decompression.
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

To get the value of a key from localhost on the default memcache port of 11211:

```
$ peekachu foo
|----- BEGIN VALUE -----|
data data data fun data data fun data data you

|-----  END VALUE  -----|
```

In the above case, peekachu is not attempting to decompress the key's value, but that's OK because as we can see the data does not appear to be compressed. if the value had been compressed we'd see something more like:

```
$ peekachu foo
|----- BEGIN VALUE -----|
youta ?fun?

|-----  END VALUE  -----|
```

To have peekachu show the decompressed value:

```
$ peekachu -z foo
|----- BEGIN VALUE -----|
data data data fun data data fun data data you

|-----  END VALUE  -----|
```

If you want to see both the raw and decompressed values for purposes of comparison:

```
$ peekachu -z -r foo
|----- BEGIN RAW VALUE -----|
youta ?fun?

|-----  END RAW VALUE  -----|

|----- BEGIN VALUE -----|
data data data fun data data fun data data you

|-----  END VALUE  -----|
```

You can use the --host and --port options to take to non-standard memcache deployments, for example if you're using something like [Twemproxy](https://github.com/twitter/twemproxy):

```
$ peekachu --host twemproxy.example.com --port 22121 -r -z foo
|----- BEGIN RAW VALUE -----|
youta ?fun?

|-----  END RAW VALUE  -----|

|----- BEGIN VALUE -----|
data data data fun data data fun data data you

|-----  END VALUE  -----|
```

Installation
============

At this point peekachu is not distributed to any of the major RPM repos, but we will try to provide pre-built binaries for each release for CentOS.

If you'd like to build your own RPM, there is a SPEC file in the repository's "scripts" directory that should work.

Failing the above, you can always build from source using the directions in the following section.

Building from Source
====================

The following instructions show how you can build peekachu from source on a CentOS 6.5 system.

Prerequisites
-------------

You will need to have the following tools installed, most of which will come in the "Development Tools" group via yum:

- autoconf
- automake
- libtool
- gcc

You should be able to get all of these via:

```
$ yum groupinstall -y 'Development tools'
```

You will also need a more recent version of libmemcached than is available in the standard repositories so install remi:

```
$ wget http://rpms.famillecollet.com/enterprise/remi-release-6.rpm
$ rpm -Uvh remi-release-6*.rpm
```

Now install libmemcached and libmemcached-devel from remi:

```
$ yum install -y cyrus-sasl-devel
$ yum --disablerepo=* --enablerepo=remi install libmemcached
$ yum --disablerepo=* --enablerepo=remi install libmemcached-devel
```

Getting the Code
----------------

You should be able to download and extract the latest stable release from Releases page.

Building and Installing
-----------------------

After downloading and extracting the code:

```
$ cd peekachu-version
$ autoreconf -fvi
$ ./configure
$ make
$ sudo make install
```

You should now be able to use peekachu.
