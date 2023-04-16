package com.daveclay.aunisoma.audio

import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.data.Buffer
import net.beadsproject.beads.ugens.ScalingMixer

class VoiceBuilder(private val audioContext: AudioContext,
                   private val mixer: ScalingMixer
) {
    fun buildNewVoice(buffer: Buffer) = Voice(audioContext, mixer, buffer)
}