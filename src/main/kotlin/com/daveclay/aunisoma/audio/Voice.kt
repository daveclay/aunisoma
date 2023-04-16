package com.daveclay.aunisoma.audio

import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.data.Buffer
import net.beadsproject.beads.ugens.Gain
import net.beadsproject.beads.ugens.Glide
import net.beadsproject.beads.ugens.ScalingMixer
import net.beadsproject.beads.ugens.WavePlayer

class Voice(audioContext: AudioContext,
            mixer: ScalingMixer,
            buffer: Buffer
) {
    val frequencyGlide = Glide(
        audioContext,
        440f,
        0f
    );

    private val wavePlayer = WavePlayer(
        audioContext,
        frequencyGlide,
        buffer
    )

    private val gainGlide = Glide(
        audioContext,
        0.5f,
        0f
    );

    val gain = Gain(
        audioContext,
        1,
        gainGlide
    )

    init {
        gain.addInput(wavePlayer)
        mixer.addInput(gain)
    }

    fun setFrequency(frequency: Float) {
        frequencyGlide.value = frequency
    }

    fun setGain(value: Float) {
        gain.gain = value
    }
}