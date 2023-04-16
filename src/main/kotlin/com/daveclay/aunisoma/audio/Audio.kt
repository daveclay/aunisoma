package com.daveclay.aunisoma.audio

import com.daveclay.aunisoma.musictheory.*
import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.ugens.Gain
import net.beadsproject.beads.ugens.Glide
import net.beadsproject.beads.ugens.ScalingMixer

/**
 * https://manualzz.com/doc/23754747/sonifying-processing--the-beads-tutorial
 */
class Audio {
    private val audioContext = AudioContext()
    private val mixer = ScalingMixer(audioContext)
    private val gainGlide = Glide(
        audioContext,
        0.5f,
        0f
    );
    private val gain = Gain(
        audioContext,
        1,
        gainGlide
    )

    private val voiceBuilder = VoiceBuilder(audioContext, mixer)

    private val voice1 = voiceBuilder.buildNewVoice()
    private val voice2 = voiceBuilder.buildNewVoice()
    private val voice3 = voiceBuilder.buildNewVoice()

    private val voices = arrayOf(
        voice1,
        voice2,
        voice3,
    )

    /*
     * A.next().next().next()
     * A.next(chord).next(chord).next(chord)
     * A.next(progression)* .next(progression)
     *
     * context.add(perspective): 12 perspectives? or one per person? Fundamentals, adding mults
     */

    fun initialize() {
        gain.addInput(mixer)
        audioContext.out.addInput(gain)
        audioContext.start()

        voice1.setFrequency(Note.C.frequencyAtOctave(3))
        voice2.setFrequency(Note.E.frequencyAtOctave(3))
        voice3.setFrequency(Note.G.frequencyAtOctave(3))
        // audioThing4.setFrequency(Note.C.frequencyAtOctave(4))
    }

    var scale = Scale.MINOR_SCALE
    var rootNote = Note.C
    var musicalKey = Key(rootNote, scale)
    var scalePosition = Scale.ScalePosition.ROOT
    var chordType = ChordType.TRIAD_CHORD

    fun update(): KeyChord {
        // Combine a Progression with a ChordType for each index in a Scale - play different chords per scale index.
        val keyChord = musicalKey.getChord(chordType, scalePosition)
        val notes = keyChord.notes

        setNotes(notes)

        return keyChord
    }

    fun setNotes(notes: List<Note>) {
        val prevFreq = 0 // open up the chord voicing by going an octave up instead of all notes in the same octave.
        val baseOctave = 2

        notes.withIndex().forEach { (index, note) ->
            if (index < voices.size) {
                val audioThing = voices[index]
                val frequencyAtOctave = note.frequencyAtOctave(baseOctave)
                if (frequencyAtOctave < prevFreq) {
                    val up = note.frequencyAtOctave(baseOctave + 1)
                    audioThing.setFrequency(up)
                } else {
                    audioThing.setFrequency(frequencyAtOctave)
                }
            }
        }
    }
}