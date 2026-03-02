# DVPE_Compiler.md

## Purpose
Translates visual block-based representations from DVPE_UI into compilable C++ code that runs on Daisy hardware.

## Current Implementation Status
⚪ **PLANNED** - Required for MVP completion

## MVP Context
- **Required for Feature**: Complete visual programming workflow
- **Priority**: High
- **Blocking**: None - dependent on DVPE_UI

## Planned Implementation Details
- **Intended Location**: Backend service or CLI tool
- **Required Interfaces**: 
  - Input: DVPE project JSON from UI
  - Output: Complete C++ source files
- **Dependencies**: DVPE_UI (provides input), libDaisy, DaisySP
- **Dependents**: DVPE_ParamControl (connects to runtime)

## Core Logic & Functionality Requirements
1. **Project Parsing**:
   - Parse DVPE project JSON
   - Validate block connections
   - Check for circular dependencies
2. **Code Generation**:
   - Generate hardware initialization code
   - Generate audio callback with block chain
   - Generate parameter control interfaces
3. **Template Management**:
   - Use templates for common patterns
   - Support DaisySP component integration
4. **Build Integration**:
   - Generate Makefile
   - Support incremental compilation

## Implementation Requirements
- **Technology**: Python, Node.js, or standalone CLI
- **Integration Points**: Receives JSON from UI, outputs C++ files
- **Data Requirements**: Block definitions, DaisySP API
- **User Experience**: Seamless compilation with error reporting

## Interface Definition (Planned)
```python
class DVPECompiler:
    def __init__(self, daisy_version: str):
        self.daisy_version = daisy_version
        
    def compile(self, project_json: dict) -> CompileResult:
        """Main compilation entry point"""
        # Parse and validate
        # Generate code
        # Return result with files or errors
        
    def validate_project(self, project_json: dict) -> ValidationResult:
        """Check project is valid before compilation"""
        
    def generate_audio_callback(self, blocks: list) -> str:
        """Generate audio processing code"""
        
    def generate_makefile(self, project_name: str) -> str:
        """Generate build file"""
```

## ARC Verification Criteria

### Functional Criteria
- [ ] Parses valid DVPE project JSON
- [ ] Detects and reports invalid configurations
- [ ] Generates compilable C++ code
- [ ] Produces working audio effect

### Input Validation Criteria  
- [ ] Validates block connections
- [ ] Detects circular dependencies
- [ ] Checks parameter ranges

### Error Handling Criteria
- [ ] Clear error messages for invalid input
- [ ] Graceful handling of unknown blocks
- [ ] Reports compilation errors with line numbers

### Quality Criteria
- [ ] Generated code follows project style guide
- [ ] Efficient code (real-time performance)
- [ ] Maintainable template system

## Implementation Notes
- Start with simple linear chain support
- Add branching support incrementally
- Consider embedded-specific optimizations
