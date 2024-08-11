<!--- This file has the version text in `idf_published_versions.txt` automatically replaced. Edit with caution. --->

This is wolfSSL version 5.7.2, Published as an Espressif Managed Component

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
idf.py create-project-from-example "gojimmypi/mywolfssl^5.7.1-Preview2f1"
cd wolfssl_benchmark
idf.py -b 115200 flash monitor
```

or for VisualGDB:

```bash
. /mnt/c/SysGCC/esp32/esp-idf/v5.1/export.sh
idf.py create-project-from-example "gojimmypi/mywolfssl^5.7.1-Preview2f1"
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


# wolfSSL Release 5.7.2 (July 08, 2024)

Release 5.7.2 has been developed according to wolfSSL's development and QA
process (see link below) and successfully passed the quality criteria.
https://www.wolfssl.com/about/wolfssl-software-development-process-quality-assurance

NOTE: * --enable-heapmath is being deprecated and will be removed by end of 2024

## Vulnerabilities
* [Medium] CVE-2024-1544
Potential ECDSA nonce side channel attack in versions of wolfSSL before 5.6.6 with wc_ecc_sign_hash calls. Generating the ECDSA nonce k samples a random number r and then truncates this randomness with a modular reduction mod n where n is the order of the elliptic curve. Analyzing the division through a control-flow revealing side-channel reveals a bias in the most significant bits of k. Depending on the curve this is either a negligible bias or a significant bias large enough to reconstruct k with lattice reduction methods. Thanks to Luca Wilke, Florian Sieck and Thomas Eisenbarth (University of Lübeck) for reporting the vulnerability. Details will appear in the proceedings of CCS 24.
Fixed https://github.com/wolfSSL/wolfssl/pull/7020


* [Medium] CVE-2024-5288
A private key blinding operation, enabled by defining the macro WOLFSSL_BLIND_PRIVATE_KEY, was added to mitigate a potential row hammer attack on ECC operations. If performing ECC private key operations in an environment where a malicious user could gain fine control over the device and perform row hammer style attacks it is recommended to update the version of wolfSSL used and to build with WOLFSSL_BLIND_PRIVATE_KEY defined. Thanks to Kemal Derya, M. Caner Tol, Berk Sunar for the report (Vernam Applied Cryptography and Cybersecurity Lab at Worcester Polytechnic Institute)
Fixed in github pull request https://github.com/wolfSSL/wolfssl/pull/7416


* [Low] When parsing a provided maliciously crafted certificate directly using wolfSSL API, outside of a TLS connection, a certificate with an excessively large number of extensions could lead to a potential DoS. There are existing sanity checks during a TLS handshake with wolfSSL which mitigate this issue. Thanks to Bing Shi for the report.
Fixed in github pull request https://github.com/wolfSSL/wolfssl/pull/7597

* [Low] CVE-2024-5991
In the function MatchDomainName(), input param str is treated as a NULL terminated string despite being user provided and unchecked. Specifically, the Openssl compatibility function X509_check_host() takes in a pointer and length to check against, with no requirements that it be NULL terminated. While calling without a NULL terminated string is very uncommon, it is still technically allowed. If a caller was attempting to do a name check on a non*NULL terminated buffer, the code would read beyond the bounds of the input array until it found a NULL terminator.
Fixed in github pull request https://github.com/wolfSSL/wolfssl/pull/7604

* [Medium] CVE-2024-5814
A malicious TLS1.2 server can force a TLS1.3 client with downgrade capability to use a ciphersuite that it did not agree to and achieve a successful connection. This is because, aside from the extensions, the client was skipping fully parsing the server hello when downgrading from TLS 1.3.
Fixed in github pull request https://github.com/wolfSSL/wolfssl/pull/7619

* [Medium] OCSP stapling version 2 response verification bypass issue when a crafted response of length 0 is received. Found with internal testing.
Fixed in github pull request https://github.com/wolfSSL/wolfssl/pull/7702

* [Medium] OCSP stapling version 2 revocation bypass with a retry of a TLS connection attempt. A revoked CA certificate could incorrectly be loaded into the trusted signers list and used in a repeat connection attempt. Found with internal testing.
Fixed in github pull request https://github.com/wolfSSL/wolfssl/pull/7702


## New Feature Additions
* Added Dilithium/ML-DSA: Implementation of ML-DSA-44/65/87 (PR 7622)
* AES RISC-V 64-bit ASM: ECB/CBC/CTR/GCM/CCM (PR 7569)
* Added CUDA support for AES encryption (PR 7436)
* Added support for gRPC (PR 7445)
* Added function wc_RsaPrivateKeyDecodeRaw to import raw RSA private keys (PR 7608)
* Added crypto callback for SHA-3 (PR 7670)
* Support for Infineon Modus Toolbox with wolfSSL (PR 7369)
* Allow user to send a user_canceled alert by calling wolfSSL_SendUserCanceled (PR 7590)
* C# wrapper SNI support added (PR 7610)
* Quantum-safe algorithm support added to the Linux kernel module (PR 7574)
* Support for NIST 800-56C Option 1 KDF, using the macro WC_KDF_NIST_SP_800_56C added (PR 7589)
* AES-XTS streaming mode added, along with hardware acceleration and kernel module use (PR 7522, 7560, 7424)
* PlatformIO FreeRTOS with ESP build and addition of benchmark and test example applications (PR 7528, 7413, 7559, 7542)


## Enhancements and Optimizations
* Expanded STM32 AES hardware acceleration support for use with STM32H5 (PR 7578)
* Adjusted wc_xmss and wc_lms settings to support use with wolfBoot (PR 7393)
* Added the --enable-rpk option to autotools build for using raw public key support (PR 7379)
* SHA-3 Thumb2, ARM32 assembly implementation added (PR 7667)
* Improvements to RSA padding to expose Pad/Unpad APIs (PR 7612)
* Updates and API additions for supporting socat version 1.8.0.0 (PR 7594)
* cmake build improvements, expanding build options with SINGLE_THREADED and post-quantum algorithms, adjusting the generation of options.h file and using “yes;no” boolean instead of strings (PR 7611, 7546, 7479, 7480, 7380)
* Improvements for Renesas RZ support (PR 7474)
* Improvements to dual algorithm certificates for post-quantum keys (PR 7286)
* Added wolfSSL_SessionIsSetup so the user can check if a session ticket has been sent by the server (PR 7430)
* hostap updates: Implement PACs for EAP-FAST and filter cipher list on TLS version change (PR 7446)
* Changed subject name comparison to match different upper and lower cases (PR 7420)
* Support for DTLS 1.3 downgrade when using PSK (PR 7367)
* Update to static memory build for more generic memory pools used (PR 7418)
* Improved performance of Kyber C implementation (PR 7654)
* Support for ECC_CACHE_CURVE with no malloc (PR 7490)
* Added the configure option --enable-debug-trace-errcodes (macro WOLFSSL_DEBUG_TRACE_ERROR_CODES) which enables more debug tracking of error code values (PR 7634)
* Enhanced wc_MakeRsaKey and wc_RsaKeyToDer to work with WOLFSSL_NO_MALLOC (PR 7362)
* Improvements to assembly implementations of ChaCha20 and Poly1305 ASM for use with MSVC (PR 7319)
* Cortex-M inline assembly labels with unique number appended (PR 7649)
* Added secret logging callback to TLS <= 1.2, enabled with the macro HAVE_SECRET_CALLBACK (PR 7372)
* Made wc_RNG_DRBG_Reseed() a public wolfCrypt API (PR 7386)
* Enabled DES3 support without the DES3 ciphers. To re-enable DES3 cipher suites, use the configure flag --enable-des3-tls-suites (PR 7315)
* Added stubs required for latest nginx (1.25.5) (PR 7449)
* Added option for using a custom salt with the function wc_ecc_ctx_set_own_salt (PR 7552)
* Added PQ files for Windows (PR 7419)
* Enhancements to static memory feature, adding the option for a global heap hint (PR 7478) and build options for a lean or debug setting, enabled with --enable-staticmemory=small or --enable-staticmemory=debug (PR 7597)
* Updated --enable-jni to define SESSION_CERTS for wolfJSSE (PR 7557)
* Exposed DTLS in Ada wrapper and updated examples (PR 7397)
* Added additional minimum TLS extension size sanity checks (PR 7602)
* ESP improvements: updating the examples and libraries, updates for Apple HomeKit SHA/SRP, and fix for endianness with SHA512 software fallback (PR 7607, 7392, 7505, 7535)
* Made the wc_CheckCertSigPubKey API publicly available with the define of the macro WOLFSSL_SMALL_CERT_VERIFY (PR 7599)
* Added an alpha/preview of additional FIPS 140-3 full submission, bringing additional algorithms such as SRTP-KDF, AES-XTS, GCM streaming, AES-CFB, ED25519, and ED448 into the FIPS module boundary (PR 7295)
* XCODE support for v5.2.3 of the FIPS module (PR 7140)
* Expanded OpenSSL compatibility layer and added EC_POINT_hex2point (PR 7191)

## Fixes
* Fixed the NXP MMCAU HW acceleration for SHA-256 (PR 7389)
* Fixed AES-CFB1 encrypt/decrypt on size (8*x-1) bits (PR 7431)
* Fixed use of %rip with SHA-256 x64 assembly (PR 7409)
* Fixed OCSP response message build for DTLS (PR 7671)
* Handled edge case in wc_ecc_mulmod() with zero (PR 7532)
* Fixed RPK (Raw Public Key) to follow certificate use correctly (PR 7375)
* Added sanity check on record header with QUIC use (PR 7638)
* Added sanity check for empty directory strings in X.509 when parsing (PR 7669)
* Added sanity check on non-conforming serial number of 0 in certificates being parsed (PR 7625)
* Fixed wolfSSL_CTX_set1_sigalgs_list() to make the TLS connection conform to the selected sig hash algorithm (PR 7693)
* Various fixes for dual algorithm certificates including small stack use and support for Certificate Signing Requests (PR 7577)
* Added sanity check for critical policy extension when wolfSSL is built without policy extension support enabled (PR 7388)
* Added sanity check that the ed25519 signature is smaller than the order (PR 7513)
* Fixed Segger emNet to handle non-blocking want read/want write (PR 7581)

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
