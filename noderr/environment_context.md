# Environment Context

Profile Name: Windows Local Embedded Audio
Environment ID: ENV-2026-02-28-windows-local-embedded
Last Updated: 2026-02-28T00:00:00Z
Validated By: AI Agent
Confidence Level: High

## Environment Metadata

environment:
  type: development
  provider: local
  lifecycle: persistent
  purpose: DaisyExamples embedded audio development
  environment_focus: DEVELOPMENT local workspace only

## Development Versus Production

development_server:
  access_urls:
    local_dev_preview:
      url: N A embedded hardware development
      description: code is built locally then flashed to hardware
      how_to_access: run build command then flash firmware
    public_deployed_app:
      url: N A no web deployment
      description: production is a flashed binary on physical Daisy hardware
      warning: DO NOT test on production hardware when validating changes

## Verified Tooling

- git available
- make available
- arm-none-eabi-gcc available
- workspace path is c:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples

## Verified Commands

- git status
- git log --oneline -3
- make --version
- arm-none-eabi-gcc --version

## Build And Flash Workflow

1. cd into target example directory
2. run make
3. flash via Daisy Web Programmer or make program-dfu
4. verify behavior on development hardware setup

## Notes

- This repository has no browser based preview URL
- Development testing happens via build plus hardware flash cycle
- Production for this project means firmware currently flashed to physical hardware
