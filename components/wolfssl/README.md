<!--- This file has the version text in `idf_published_versions.txt` automatically replaced. Edit with caution. --->

This is the Espressif Component Version of [v5.6.6-stable](https://github.com/wolfSSL/wolfssl/releases/tag/v5.6.6-stable), 
Staging Preview (version ^5.7.0).

NOTE: This is a slightly-modified version that includes:
- [PR #7077](https://github.com/wolfSSL/wolfssl/pull/7077) wolfSSL_NewThread() type update for Espressif FreeRTOS. [Patch](https://patch-diff.githubusercontent.com/raw/wolfSSL/wolfssl/pull/7081.patch)
- [PR #7081](https://github.com/wolfSSL/wolfssl/pull/7081) Add wolfcrypt SHA support for ESP32-C2/ESP8684. [Patch](https://patch-diff.githubusercontent.com/raw/wolfSSL/wolfssl/pull/7081.patch)

For questions or beta test of this library, please send a message to support@wolfssl.com

For details on how wolfSSL is published to the ESP Component Registry, see the
[INSTALL.md](https://github.com/wolfssl/wolfssl/tree/master/IDE/Espressif/component-manager/INSTALL.md) file.

See the [wolfSSL Manual](https://www.wolfssl.com/documentation/manuals/wolfssl/index.html).

The [wolfSSL embedded TLS library](https://www.wolfssl.com/products/wolfssl/) is a lightweight, portable, 
C-language-based SSL/TLS library targeted at IoT, embedded, and RTOS environments primarily because of its size, 
speed, and feature set. It works seamlessly in desktop, enterprise, and cloud environments as well. 
wolfSSL supports industry standards up to the current [TLS 1.3](https://www.wolfssl.com/tls13) and DTLS 1.3, 
is up to 20 times smaller than OpenSSL, offers a simple API, an OpenSSL compatibility layer,
OCSP and CRL support, is backed by the robust [wolfCrypt cryptography library](https://github.com/wolfssl/wolfssl/tree/master/wolfcrypt), 
and much more.

The CMVP has issued FIPS 140-2 Certificates #3389 and #2425 for the wolfCrypt Module developed by wolfSSL Inc.  
For more information, see our [FIPS FAQ](https://www.wolfssl.com/license/fips/) or contact fips@wolfssl.com.

# Getting Started

Check out the Examples on the right pane of the [wolfssl component page](https://components.espressif.com/components/wolfssl/wolfssl/).

Typically you need only 4 lines to run an example from scratch in the EDP-IDF environment:

```bash
. ~/esp/esp-idf/export.sh
idf.py create-project-from-example "gojimmypi/mywolfssl^5.7.0"
cd wolfssl_benchmark
idf.py -b 115200 flash monitor
```

or for VisualGDB:

```bash
. /mnt/c/SysGCC/esp32/esp-idf/v5.1/export.sh
idf.py create-project-from-example "gojimmypi/mywolfssl^5.7.0"
cd wolfssl_benchmark
idf.py -b 115200 flash monitor
```


### Espressif Component Notes

Here are some ESP Registry-specific details of the wolfssl component.

#### Component Name

The naming convention of the build-system name of a dependency installed by the component manager
is always `namespace__component`. The namespace for wolfSSL is `wolfssl`. The build-system name
is thus `wolfssl__wolfssl`. We'll soon be publishing `wolfssl__wolfssh`, `wolfssl__wolfmqtt` and more.

A project `cmakelists.txt` doesn't need to mention it at all when using wolfSSL as a managed component.


#### Component Manager

To check which version of the [Component Manager](https://docs.espressif.com/projects/idf-component-manager/en/latest/getting_started/index.html#checking-the-idf-component-manager-version) 
is currently available, use the command:

```
python -m idf_component_manager -h
```

The Component Manager should have been installed during the [installation of the ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#installation). 
If your version of ESP-IDF doesn't come with the IDF Component Manager, 
you can [install it](https://docs.espressif.com/projects/idf-component-manager/en/latest/guides/updating_component_manager.html#installing-and-updating-the-idf-component-manager):

```
python -m pip install --upgrade idf-component-manager
```

For further details on the Espressif Component Manager, see the [idf-component-manager repo](https://github.com/espressif/idf-component-manager/).

#### Contact

Have a specific request or questions? We'd love to hear from you! Please contact us at 
[support@wolfssl.com](mailto:support@wolfssl.com?subject=Espressif%20Component%20Question) or 
[open an issue on GitHub](https://github.com/wolfSSL/wolfssl/issues/new/choose).

# Licensing and Support

wolfSSL (formerly known as CyaSSL) and wolfCrypt are either licensed for use
under the GPLv2 (or at your option any later version) or a standard commercial
license. For our users who cannot use wolfSSL under GPLv2
(or any later version), a commercial license to wolfSSL and wolfCrypt is
available. 

See the [LICENSE.txt](./LICENSE.txt), visit [wolfssl.com/license](https://www.wolfssl.com/license/),
contact us at [licensing@wolfssl.com](mailto:licensing@wolfssl.com?subject=Espressif%20Component%20License%20Question) 
or call +1 425 245 8247

View Commercial Support Options: [wolfssl.com/products/support-and-maintenance](https://www.wolfssl.com/products/support-and-maintenance/)
# wolfSSL Embedded SSL/TLS Library

The [wolfSSL embedded SSL library](https://www.wolfssl.com/products/wolfssl/) 
(formerly CyaSSL) is a lightweight SSL/TLS library written in ANSI C and
targeted for embedded, RTOS, and resource-constrained environments - primarily
because of its small size, speed, and feature set.  It is commonly used in
standard operating environments as well because of its royalty-free pricing
and excellent cross platform support. wolfSSL supports industry standards up
to the current [TLS 1.3](https://www.wolfssl.com/tls13) and DTLS 1.3, is up to
20 times smaller than OpenSSL, and offers progressive ciphers such as ChaCha20,
Curve25519, Blake2b and Post-Quantum TLS 1.3 groups. User benchmarking and
feedback reports dramatically better performance when using wolfSSL over
OpenSSL.

wolfSSL is powered by the wolfCrypt cryptography library. Two versions of
wolfCrypt have been FIPS 140-2 validated (Certificate #2425 and
certificate #3389). FIPS 140-3 validation is in progress. For additional
information, visit the [wolfCrypt FIPS FAQ](https://www.wolfssl.com/license/fips/)
or contact fips@wolfssl.com.

## Why Choose wolfSSL?

There are many reasons to choose wolfSSL as your embedded, desktop, mobile, or
enterprise SSL/TLS solution. Some of the top reasons include size (typical
footprint sizes range from 20-100 kB), support for the newest standards
(SSL 3.0, TLS 1.0, TLS 1.1, TLS 1.2, TLS 1.3, DTLS 1.0, DTLS 1.2, and DTLS 1.3),
current and progressive cipher support (including stream ciphers), multi-platform,
royalty free, and an OpenSSL compatibility API to ease porting into existing
applications which have previously used the OpenSSL package. For a complete
feature list, see [Chapter 4](https://www.wolfssl.com/docs/wolfssl-manual/ch4/)
of the wolfSSL manual.

## Notes, Please Read

### Note 1
wolfSSL as of 3.6.6 no longer enables SSLv3 by default.  wolfSSL also no longer
supports static key cipher suites with PSK, RSA, or ECDH. This means if you
plan to use TLS cipher suites you must enable DH (DH is on by default), or
enable ECC (ECC is on by default), or you must enable static key cipher suites
with one or more of the following defines:

```
WOLFSSL_STATIC_DH
WOLFSSL_STATIC_RSA
WOLFSSL_STATIC_PSK
```
Though static key cipher suites are deprecated and will be removed from future
versions of TLS.  They also lower your security by removing PFS.

When compiling `ssl.c`, wolfSSL will now issue a compiler error if no cipher
suites are available. You can remove this error by defining
`WOLFSSL_ALLOW_NO_SUITES` in the event that you desire that, i.e., you're
not using TLS cipher suites.

### Note 2
wolfSSL takes a different approach to certificate verification than OpenSSL
does. The default policy for the client is to verify the server, this means
that if you don't load CAs to verify the server you'll get a connect error,
no signer error to confirm failure (-188).

If you want to mimic OpenSSL behavior of having `SSL_connect` succeed even if
verifying the server fails and reducing security you can do this by calling:

```c
wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);
```

before calling `wolfSSL_new();`. Though it's not recommended.

### Note 3
The enum values SHA, SHA256, SHA384, SHA512 are no longer available when
wolfSSL is built with `--enable-opensslextra` (`OPENSSL_EXTRA`) or with the
macro `NO_OLD_SHA_NAMES`. These names get mapped to the OpenSSL API for a
single call hash function. Instead the name `WC_SHA`, `WC_SHA256`, `WC_SHA384` and
`WC_SHA512` should be used for the enum name.


# wolfSSL Release 5.7.0 (Mar 20, 2024)

Release 5.7.0 has been developed according to wolfSSL's development and QA
process (see link below) and successfully passed the quality criteria.
https://www.wolfssl.com/about/wolfssl-software-development-process-quality-assurance

NOTE: * --enable-heapmath is being deprecated and will be removed by end of 2024

NOTE: In future releases, --enable-des3 (which is disabled by default) will be insufficient in itself to enable DES3 in TLS cipher suites. A new option, --enable-des3-tls-suites, will need to be supplied in addition.  This option should only be used in backward compatibility scenarios, as it is inherently insecure.

NOTE: This release switches the default ASN.1 parser to the new ASN template code. If the original ASN.1 code is preferred define `WOLFSSL_ASN_ORIGINAL` to use it. See PR #7199.

## Vulnerabilities
* [High] CVE-2024-0901 Potential denial of service and out of bounds read. Affects TLS 1.3 on the server side when accepting a connection from a malicious TLS 1.3 client. If using TLS 1.3 on the server side it is recommended to update the version of wolfSSL used. Fixed in this GitHub pull request https://github.com/wolfSSL/wolfssl/pull/7099


* [Med] CVE-2024-1545 Fault Injection vulnerability in RsaPrivateDecryption function that potentially allows an attacker that has access to the same system with a victims process to perform a Rowhammer fault injection. Thanks to Junkai Liang, Zhi Zhang, Xin Zhang, Qingni Shen for the report (Peking University, The University of Western Australia)."
Fixed in this GitHub pull request https://github.com/wolfSSL/wolfssl/pull/7167


* [Med] Fault injection attack with EdDSA signature operations. This affects ed25519 sign operations where the system could be susceptible to Rowhammer attacks. Thanks to Junkai Liang, Zhi Zhang, Xin Zhang, Qingni Shen for the report (Peking University, The University of Western Australia).
Fixed in this GitHub pull request https://github.com/wolfSSL/wolfssl/pull/7212


## New Feature Additions

* Added --enable-experimental configure flag to gate out features that are currently experimental. Now liboqs, kyber, lms, xmss, and dual-alg-certs require the --enable-experimental flag.

### POST QUANTUM SUPPORT ADDITIONS
* Experimental framework for using wolfSSL’s XMSS implementation (PR 7161)
* Experimental framework for using wolfSSL’s LMS implementation (PR 7283)
* Experimental wolfSSL Kyber implementation and assembly optimizations, enabled with --enable-experimental --enable-kyber (PR 7318)
* Experimental support for post quantum dual key/signature certificates. A few known issues and sanitizer checks are in progress with this feature. Enabled with the configure flags --enable-experimental --enable-dual-alg-certs (PR 7112)
* CryptoCb support for PQC algorithms (PR 7110)

### OTHER FEATURE ADDITIONS
* The Linux kernel module now supports registration of AES-GCM, AES-XTS, AES-CBC, and AES-CFB with the kernel cryptosystem through the new --enable-linuxkm-lkcapi-register option, enabling automatic use of wolfCrypt implementations by the dm-crypt/luks and ESP subsystems.  In particular, wolfCrypt AES-XTS with –enable-aesni is faster than the native kernel implementation.
* CryptoCb hook to one-shot CMAC functions (PR 7059)
* BER content streaming support for PKCS7_VerifySignedData and sign/encrypt operations (PR 6961 & 7184)
* IoT-Safe SHA-384 and SHA-512 support (PR 7176)
* I/O callbacks for content and output with PKCS7 bundle sign/encrypt to reduce peak memory usage (PR 7272)
* Microchip PIC24 support and example project (PR 7151)
* AutoSAR shim layer for RNG, SHA256, and AES (PR 7296)
* wolfSSL_CertManagerUnloadIntermediateCerts API to clear intermediate certs added to certificate store (PR 7245)
* Implement SSL_get_peer_signature_nid and SSL_get_peer_signature_type_nid (PR 7236)


## Enhancements and Optimizations

* Remove obsolete user-crypto functionality and Intel IPP support (PR 7097)
* Support for RSA-PSS signatures with CRL use (PR 7119)
* Enhancement for AES-GCM use with Xilsecure on Microblaze (PR 7051)
* Support for crypto cb only build with ECC and NXP CAAM (PR 7269)
* Improve liboqs integration adding locking and init/cleanup functions (PR 7026)
* Prevent memory access before clientSession->serverRow and clientSession->serverIdx are sanitized (PR 7096)
* Enhancements to reproducible build (PR 7267)
* Update Arduino example TLS Client/Server and improve support for ESP32 (PR 7304 & 7177)
* XC32 compiler version 4.x compatibility (PR 7128)
* Porting for build on PlayStation 3 and 4 (PR 7072)
* Improvements for Espressif use; SHA HW/SW selection and use on ESP32-C2/ESP8684, wolfSSL_NewThread() type, component cmake fix, and update TLS client example for ESP8266 (PR 7081, 7173, 7077, 7148, 7240)
* Allow crypto callbacks with SHA-1 HW (PR 7087)
* Update OpenSSH port to version 9.6p1(PR 7203)
* ARM Thumb2 enhancements, AES-GCM support for GCM_SMALL, alignment fix on key, fix for ASM clobber list (PR 7291,7301,7221)
* Expand heap hint support for static memory build with more x509 functions (PR 7136)
* Improving ARMv8 ChaCha20 ASM (alignment) (PR 7182)
* Unknown extension callback wolfSSL_CertManagerSetUnknownExtCallback added to CertManager (PR 7194)
* Implement wc_rng_new_ex for use with devID’s with crypto callback (PR 7271)
* Allow reading 0-RTT data after writing 0.5-RTT data (PR 7102)
* Send alert on bad PSK binder error (PR 7235)
* Enhancements to CMake build files for use with cross compiling (PR 7188)


## Fixes

* Fix for checking result of MAC verify when no AAD is used with AES-GCM and Xilinx Xilsecure (PR 7051)
* Fix for Aria sign use (PR 7082)
* Fix for invalid `dh_ffdhe_test` test case using Intel QuickAssist (PR 7085)
* Fixes for TI AES and SHA on TM4C with HW acceleration and add full AES GCM and CCM support with TLS (PR 7018)
* Fixes for STM32 PKA use with ECC (PR 7098)
* Fixes for TLS 1.3 with crypto callbacks to offload KDF / HMAC operation (PR 7070)
* Fix include path for FSP 3.5 on Renesas RA6M4 (PR 7101)
* Siphash x64 asm fix for use with older compilers (PR 7299)
* Fix for SGX build with SP (PR 7308)
* Fix to Make it mandatory that the cookie is sent back in new ClientHello when seen in a HelloRetryRequest with (PR 7190)
* Fix for wrap around behavior with BIO pairs (PR 7169)
* OCSP fixes for parsing of response correctly when there was a revocation reason and returning correct error value with date checks (PR 7241 & 7255)
* Fix build with `NO_STDIO_FILESYSTEM` and improve checks for `XGETENV` (PR 7150)
* Fix for DTLS sequence number and cookie when downgrading DTLS version (PR 7214)
* Fix for write_dup use with chacha-poly cipher suites (PR 7206)
* Fix for multiple handshake messages in one record failing with OUT_OF_ORDER_E when downgrading from TLS 1.3 to TLS 1.2 (PR 7141)
* Fix for AES ECB build with Thumb and alignment (PR 7094)
* Fix for negotiate handshake until the end in wolfSSL_read/wolfSSL_write if hitting an edge case with want read/write (PR 7237)

For additional vulnerability information visit the vulnerability page at:
https://www.wolfssl.com/docs/security-vulnerabilities/

See INSTALL file for build instructions.
More info can be found on-line at: https://wolfssl.com/wolfSSL/Docs.html

# Resources

[wolfSSL Website](https://www.wolfssl.com/)

[wolfSSL Wiki](https://github.com/wolfSSL/wolfssl/wiki)

[FIPS 140-2/140-3 FAQ](https://wolfssl.com/license/fips)

[wolfSSL Documentation](https://wolfssl.com/wolfSSL/Docs.html)

[wolfSSL Manual](https://wolfssl.com/wolfSSL/Docs-wolfssl-manual-toc.html)

[wolfSSL API Reference](https://wolfssl.com/wolfSSL/Docs-wolfssl-manual-17-wolfssl-api-reference.html)

[wolfCrypt API Reference](https://wolfssl.com/wolfSSL/Docs-wolfssl-manual-18-wolfcrypt-api-reference.html)

[TLS 1.3](https://www.wolfssl.com/docs/tls13/)

[wolfSSL Vulnerabilities](https://www.wolfssl.com/docs/security-vulnerabilities/)

[Additional wolfSSL Examples](https://github.com/wolfssl/wolfssl-examples)

# Directory structure

```
<wolfssl_root>
├── certs   [Certificates used in tests and examples]
├── cmake   [Cmake build utilities]
├── debian  [Debian packaging files]
├── doc     [Documentation for wolfSSL (Doxygen)]
├── Docker  [Prebuilt Docker environments]
├── examples    [wolfSSL examples]
│   ├── asn1    [ASN.1 printing example]
│   ├── async   [Asynchronous Cryptography example]
│   ├── benchmark   [TLS benchmark example]
│   ├── client  [Client example]
│   ├── configs [Example build configurations]
│   ├── echoclient  [Echoclient example]
│   ├── echoserver  [Echoserver example]
│   ├── pem [Example for convert between PEM and DER]
│   ├── sctp    [Servers and clients that demonstrate wolfSSL's DTLS-SCTP support]
│   └── server  [Server example]
├── IDE     [Contains example projects for various development environments]
├── linuxkm [Linux Kernel Module implementation]
├── m4      [Autotools utilities]
├── mcapi   [wolfSSL MPLAB X Project Files]
├── mplabx  [wolfSSL MPLAB X Project Files]
├── mqx     [wolfSSL Freescale CodeWarrior Project Files]
├── rpm     [RPM packaging metadata]
├── RTOS
│   └── nuttx   [Port of wolfSSL for NuttX]
├── scripts [Testing scripts]
├── src     [wolfSSL source code]
├── sslSniffer  [wolfSSL sniffer can be used to passively sniff SSL traffic]
├── support [Contains the pkg-config file]
├── tests   [Unit and configuration testing]
├── testsuite   [Test application that orchestrates tests]
├── tirtos  [Port of wolfSSL for TI RTOS]
├── wolfcrypt   [The wolfCrypt component]
│   ├── benchmark   [Cryptography benchmarking application]
│   ├── src         [wolfCrypt source code]
│   │   └── port    [Supported hardware acceleration ports]
│   └── test        [Cryptography testing application]
├── wolfssl [Header files]
│   ├── openssl [Compatibility layer headers]
│   └── wolfcrypt   [Header files]
├── wrapper [wolfSSL language wrappers]
└── zephyr  [Port of wolfSSL for Zephyr RTOS]
```
