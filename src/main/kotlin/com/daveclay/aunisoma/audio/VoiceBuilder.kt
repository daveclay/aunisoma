package com.daveclay.aunisoma.audio

import com.daveclay.aunisoma.audio.Voice
import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.ugens.ScalingMixer

class VoiceBuilder(private val audioContext: AudioContext,
                   private val mixer: ScalingMixer
) {
    fun buildNewVoice() = Voice(audioContext, mixer)
}