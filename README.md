# Google Chrome Elevation Service Privilege Elevation Exploit
![image](https://github.com/user-attachments/assets/c050f4df-4012-4b69-aff0-e8fe3684a31a)

## Intro
The Chrome Elevation Service uses the `RunRecoveryCRXElevated` method that can be abused by low privileged users to copy **any** file on a host.

This vulnerability is very old (2019) and the original (old) PoC isnt mine.

I wrote my PoC just to reimagine this vuln.

# Refs
- https://issues.chromium.org/issues/40095920
- https://source.chromium.org/chromium/chromium/src/+/main:chrome/elevation_service/elevation_service_idl.idl
- https://source.chromium.org/chromium/chromium/src/+/main:chrome/elevation_service/elevated_recovery_impl.cc
- https://source.chromium.org/chromium/chromium/src/+/main:chrome/elevation_service/elevator.h
