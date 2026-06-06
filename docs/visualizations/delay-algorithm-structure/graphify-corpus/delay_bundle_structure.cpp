// Graphify seed file for the delay algorithm visualization package.
// This file is not firmware. It mirrors the current source names and call
// relationships so Graphify can build an AST graph without an LLM API key.

namespace delay_visualization_graph
{

enum class DaisyDelayFxSource
{
    kMultiFxPedal,
    kReverbPlayground,
    kFunBox,
    kSdramDelaylines,
};

struct FieldDelayFieldApp
{
    void RunConfiguredFieldDelayProject()
    {
        ShowStartupDisplay();
        PrimeControls();
        StartAudio();
        MainLoop();
    }

    void MainLoop()
    {
        ProcessControls();
        UpdateDisplay();
        UpdateLeds();
    }

    void ProcessControls()
    {
        ProcessLayeredKnobs();
        ProcessAKeys();
        ProcessBKeys();
        ProcessMidi();
    }

    void ProcessLayeredKnobs()
    {
        CurrentLayer();
        EnterLayer();
        SetParameterValue();
        RecordZoom();
    }

    void ProcessAKeys()
    {
        SelectBundleAlgorithm();
        SetInternalSynthMode();
        SetInternalSynthHoldMode();
        RecordAlgorithmZoom();
    }

    void SelectBundleAlgorithm()
    {
        SaveBundleSnapshot();
        SetSource();
        RestoreBundleSnapshot();
        EnterLayer();
        RecordAlgorithmZoom();
    }

    void ProcessBKeys()
    {
        KeyboardIndexForPhysicalKey();
        TriggerFieldKeyAction();
        RecordZoom();
    }

    void ProcessMidi()
    {
        HandleMidiEvent();
    }

    void UpdateDisplay()
    {
        DrawMain();
        DrawZoom();
    }

    void UpdateLeds()
    {
        BundleFieldKeyLedValues();
        SwapBuffersAndTransmit();
    }

    void ShowStartupDisplay() {}
    void PrimeControls() {}
    void StartAudio() {}
    void CurrentLayer() {}
    void EnterLayer() {}
    void SetParameterValue() {}
    void RecordZoom() {}
    void SetInternalSynthMode() {}
    void SetInternalSynthHoldMode() {}
    void RecordAlgorithmZoom() {}
    void SaveBundleSnapshot() {}
    void RestoreBundleSnapshot() {}
    void SetSource() {}
    void KeyboardIndexForPhysicalKey() {}
    void TriggerFieldKeyAction() {}
    void HandleMidiEvent() {}
    void DrawMain() {}
    void DrawZoom() {}
    void BundleFieldKeyLedValues() {}
    void SwapBuffersAndTransmit() {}
};

struct DaisyDelayFxCore
{
    DaisyDelayFxSource source = DaisyDelayFxSource::kMultiFxPedal;

    void Prepare()
    {
        AttachDelayStorage();
        RebuildParameters();
        ResetToDefaultState();
    }

    void Process()
    {
        ProcessInternalSynth();
        DispatchSelectedAlgorithm();
        MixDryWetOutput();
    }

    void DispatchSelectedAlgorithm()
    {
        if(source == DaisyDelayFxSource::kReverbPlayground)
        {
            ProcessReverbPlayground();
        }
        else if(source == DaisyDelayFxSource::kFunBox)
        {
            ProcessFunBox();
        }
        else if(source == DaisyDelayFxSource::kSdramDelaylines)
        {
            ProcessSdramDelaylines();
        }
        else
        {
            ProcessMultiFx();
        }
    }

    void RebuildParameters()
    {
        AddBaseLayerParameters();
        AddShiftOneParameters();
        AddShiftTwoParameters();
        UpdateParameterCache();
    }

    void ProcessInternalSynth()
    {
        FindVoiceForNote();
        AllocateVoice();
        ProcessSynthVoice();
    }

    void ProcessMultiFx()
    {
        SmoothDelayTime();
        ReadStereoDelayLines();
        ApplyFlutterLfo();
        ApplyFeedbackTone();
        ApplyFastTanhGrit();
        WriteFreezeAwareDelayLines();
    }

    void ProcessReverbPlayground()
    {
        ReadFourTankDelays();
        ApplyDamping();
        ApplyDiffusion();
        ApplyHadamardFeedbackMatrix();
        WriteFourTankDelays();
        ApplyStereoWidth();
    }

    void ProcessFunBox()
    {
        ReadNormalTaps();
        ReadGrainTap();
        ReadReverseAccentTap();
        BlendTexture();
        WriteCrossFeedback();
    }

    void ProcessSdramDelaylines()
    {
        SmoothLongDelayTime();
        ReadPrimaryFractionalTaps();
        ReadSecondaryWarpTaps();
        ApplyPingPongFeedback();
        ApplySmearBlend();
    }

    void AttachDelayStorage() {}
    void ResetToDefaultState() {}
    void MixDryWetOutput() {}
    void AddBaseLayerParameters() {}
    void AddShiftOneParameters() {}
    void AddShiftTwoParameters() {}
    void UpdateParameterCache() {}
    void FindVoiceForNote() {}
    void AllocateVoice() {}
    void ProcessSynthVoice() {}
    void SmoothDelayTime() {}
    void ReadStereoDelayLines() {}
    void ApplyFlutterLfo() {}
    void ApplyFeedbackTone() {}
    void ApplyFastTanhGrit() {}
    void WriteFreezeAwareDelayLines() {}
    void ReadFourTankDelays() {}
    void ApplyDamping() {}
    void ApplyDiffusion() {}
    void ApplyHadamardFeedbackMatrix() {}
    void WriteFourTankDelays() {}
    void ApplyStereoWidth() {}
    void ReadNormalTaps() {}
    void ReadGrainTap() {}
    void ReadReverseAccentTap() {}
    void BlendTexture() {}
    void WriteCrossFeedback() {}
    void SmoothLongDelayTime() {}
    void ReadPrimaryFractionalTaps() {}
    void ReadSecondaryWarpTaps() {}
    void ApplyPingPongFeedback() {}
    void ApplySmearBlend() {}
};

} // namespace delay_visualization_graph
