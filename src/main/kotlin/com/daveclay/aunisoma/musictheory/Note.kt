package com.daveclay.aunisoma.musictheory

import java.lang.IllegalArgumentException

class Note(val name: String, val frequencies: Array<Float>) {

    companion object {
        fun findByString(noteString: String): Note {
            return when(noteString) {
                "A" -> Note.A
                "A#" -> Note.A_SHARP
                "B" -> Note.B
                "C" -> Note.C
                "C#" -> Note.C_SHARP
                "D" -> Note.D
                "D#" -> Note.D_SHARP
                "E" -> Note.E
                "F" -> Note.F
                "F#" -> Note.F_SHARP
                "G" -> Note.G
                "G#" -> Note.G_SHARP
                else -> throw IllegalArgumentException("No clue what note $noteString is pal")
            }
        }

        val C = Note(
            "C", arrayOf(
                65.41f, 130.81f, 261.63f, 523.25f, 1046.50f, 2093.00f, 4186.01f
            )
        )
        val C_SHARP = Note(
            "C#", arrayOf(
                69.30f, 138.59f, 277.18f, 554.37f, 1108.73f, 2217.46f, 4434.92f
            )
        )
        val D = Note(
            "D", arrayOf(
                73.42f, 146.83f, 293.66f, 587.33f, 1174.66f, 2349.32f, 4698.63f
            )
        )
        val D_SHARP = Note(
            "D#", arrayOf(
                77.78f, 155.56f, 311.13f, 622.25f, 1244.51f, 2489.02f, 4978.03f
            )
        )
        val E = Note(
            "E", arrayOf(
                82.41f, 164.81f, 329.63f, 659.25f, 1318.51f, 2637.02f, 5274.04f
            )
        )
        val F = Note(
            "F", arrayOf(
                87.31f, 174.61f, 349.23f, 698.46f, 1396.91f, 2793.83f, 5587.65f
            )
        )
        val F_SHARP = Note(
            "F#", arrayOf(
                92.50f, 185.00f, 369.99f, 739.99f, 1479.98f, 2959.96f, 5919.91f
            )
        )
        val G = Note(
            "G", arrayOf(
                98.00f, 196.00f, 392.00f, 783.99f, 1567.98f, 3135.96f, 6271.93f
            )
        )
        val G_SHARP = Note(
            "G#", arrayOf(
                103.83f, 207.65f, 415.30f, 830.61f, 1661.22f, 3322.44f, 6644.88f
            )
        )
        val A = Note(
            "A", arrayOf(
                110.00f, 220.00f, 440.00f, 880.00f, 1760.00f, 3520.00f, 7040.00f
            )
        )
        val A_SHARP = Note(
            "A#", arrayOf(
                116.54f, 233.08f, 466.16f, 932.33f, 1864.66f, 3729.31f, 7458.62f
            )
        )
        val B = Note(
            "B", arrayOf(
                123.47f, 246.94f, 493.88f, 987.77f, 1975.53f, 3951.07f, 7902.13f
            )
        )

        val NOTES = Notes(
            arrayOf(
                C,
                C_SHARP,
                D,
                D_SHARP,
                E,
                F,
                F_SHARP,
                G,
                G_SHARP,
                A,
                A_SHARP,
                B
            )
        )
    }

    val size = frequencies.size
    fun frequencyAtOctave(octave: Int) = frequencies[octave]
    fun indexOfNote() = NOTES.indexOf(this)

    override fun toString() = name
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as Note

        if (name != other.name) return false
        if (!frequencies.contentEquals(other.frequencies)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = name.hashCode()
        result = 31 * result + frequencies.contentHashCode()
        return result
    }
}