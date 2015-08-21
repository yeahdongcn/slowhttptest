In version 1.1 we added test for Apache range header handling vulnerability, described in [CVE-2011-3192](http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2011-3192).

-R would start the test with default values, and generate a HEAD request with header Range: 0-, x-1, x-2, x-3 ... x-y

where x is set by -a argument, and y is set by -b argument and increments in step of 1 byte.

Don't run the scan from same machine where server under test is, because entire system might become unstable.

Example:
```
slowhttptest -R -u http://127.0.1.1/ -t HEAD -c 1000 -a 10 -b 3000 -r 500
```

Test can be performed with various connection rate and number, over SSL, etc. Basically all options that make sense to use with range test are supported in -R mode and are described [here](InstallationAndUsage.md) .