
#ifndef _AUDIO_CAPTURE_H_
#define _AUDIO_CAPTURE_H_
#pragma once

#include "common.h"
#include "base/types.h"

class CAudioCapture;
typedef void (CALLBACK *AudioCaptureCallbackProc)(uint8 *data, ulong length, void *user_data);

class CAudioCapture : public CCallbackAble<AudioCaptureCallbackProc>
{
protected:
	AudioFormat _format;

	// inputSamples * 96
	ulong _bufferLength;
	bool _Initialize(AudioFormat *format);

public:
	CAudioCapture(ulong bufferLength);
	virtual ~CAudioCapture();

	virtual bool Initialize(AudioFormat *format) = 0;
	virtual void Shutdown() = 0;
	virtual bool Start() = 0;
	virtual void Stop() = 0;
};

#endif