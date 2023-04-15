package musictheory

import musictheory.Note.Companion.NOTES

class Note(val name: String, val frequencies: Array<Float>) {

    companion object {
        val C = Note("C", arrayOf(
            65.41f, 130.81f, 261.63f, 523.25f, 1046.50f, 2093.00f, 4186.01f))
        val C_SHARP = Note("C#", arrayOf(
            69.30f, 138.59f, 277.18f, 554.37f, 1108.73f, 2217.46f, 4434.92f))
        val D = Note("D", arrayOf(
            73.42f, 146.83f, 293.66f, 587.33f, 1174.66f, 2349.32f, 4698.63f))
        val D_SHARP = Note("D#", arrayOf(
            77.78f, 155.56f, 311.13f, 622.25f, 1244.51f, 2489.02f, 4978.03f))
        val E = Note("E", arrayOf(
            82.41f, 164.81f, 329.63f, 659.25f, 1318.51f, 2637.02f, 5274.04f))
        val F = Note("F", arrayOf(
            87.31f, 174.61f, 349.23f, 698.46f, 1396.91f, 2793.83f, 5587.65f))
        val F_SHARP = Note("F#", arrayOf(
            92.50f, 185.00f, 369.99f, 739.99f, 1479.98f, 2959.96f, 5919.91f))
        val G = Note("G", arrayOf(
            98.00f, 196.00f, 392.00f, 783.99f, 1567.98f, 3135.96f, 6271.93f))
        val G_SHARP = Note("G#", arrayOf(
            103.83f, 207.65f, 415.30f, 830.61f, 1661.22f, 3322.44f, 6644.88f))
        val A = Note("A", arrayOf(
            110.00f, 220.00f, 440.00f, 880.00f, 1760.00f, 3520.00f, 7040.00f))
        val A_SHARP = Note("A#", arrayOf(
            116.54f, 233.08f, 466.16f, 932.33f, 1864.66f, 3729.31f, 7458.62f))
        val B = Note("B", arrayOf(
            123.47f, 246.94f, 493.88f, 987.77f, 1975.53f, 3951.07f, 7902.13f))

        val NOTES = Notes(arrayOf(
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
        ))
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

class Notes(val allNotes: Array<Note>) {
    val size = allNotes.size

    fun noteAt(index: Int): Note {
        val boundedIndex = index % size
        return allNotes[boundedIndex]
    }

    fun indexOf(note: Note): Int = allNotes.indexOf(note)
}

enum class Interval(val index: Int) {
    UNISON(0),
    MINOR_SECOND(1),
    MAJOR_SECOND(2),
    MINOR_THIRD(3),
    MAJOR_THIRD(4),
    FOURTH(5),
    FLAT_FIFTH(6),
    FIFTH(7),
    MINOR_SIXTH(8),
    MAJOR_SIXTH(9),
    MINOR_SEVENTH(10),
    MAJOR_SEVENTH(11),
    OCTAVE(12),
    ROOT(UNISON.index),
    DIMINISHED_FIFTH(FLAT_FIFTH.index),
    TRITONE(FLAT_FIFTH.index),
    DIMINISHED_SEVENTH(MAJOR_SIXTH.index);

    fun fromNote(rootNote: Note): Note {
        return NOTES.noteAt(rootNote.indexOfNote() + index)
    }
}

enum class Scale(val display: String, vararg val intervals: Interval) {
    UNISON_SCALE( "Unison", Interval.UNISON),
    PERFECT_SCALE( "Perfect", Interval.UNISON, Interval.FOURTH, Interval.FIFTH),
    MINOR_PENTATONIC_SCALE( "Minor Pentatonic", Interval.UNISON, Interval.MINOR_THIRD, Interval.FOURTH, Interval.FIFTH, Interval.MINOR_SEVENTH),
    MAJOR_PENTATONIC_SCALE( "Major Pentatonic", Interval.UNISON, Interval.MAJOR_THIRD, Interval.FOURTH, Interval.FIFTH, Interval.MAJOR_SEVENTH),
    BLUES_CHROMATIC_SCALE( "Blues Chromatic", Interval.UNISON, Interval.MINOR_THIRD, Interval.MAJOR_THIRD, Interval.FOURTH, Interval.FLAT_FIFTH, Interval.FIFTH, Interval.MINOR_SEVENTH),
    BLUES_MAJOR_SCALE( "Blues Major", Interval.UNISON, Interval.MAJOR_THIRD, Interval.FOURTH, Interval.FLAT_FIFTH, Interval.FIFTH, Interval.MAJOR_SEVENTH),
    BLUES_MINOR_SCALE( "Blues Minor", Interval.UNISON, Interval.MINOR_THIRD, Interval.FOURTH, Interval.FLAT_FIFTH, Interval.FIFTH, Interval.MINOR_SEVENTH),
    MINOR_SCALE( "Minor", Interval.UNISON, Interval.MAJOR_SECOND, Interval.MINOR_THIRD, Interval.FOURTH, Interval.FIFTH, Interval.MINOR_SIXTH, Interval.MINOR_SEVENTH),
    MAJOR_SCALE( "Major", Interval.UNISON, Interval.MAJOR_SECOND, Interval.MAJOR_THIRD, Interval.FOURTH, Interval.FIFTH, Interval.MAJOR_SIXTH, Interval.MAJOR_SEVENTH);

    enum class ScalePosition(val scaleIntervalIndex: Int) {
        // Which Interval in the Scale?
        ROOT(0),
        SECOND(1),
        THIRD(2),
        FOURTH(3),
        FIFTH(4),
        SIXTH(5),
        SEVENTH(6);
    }

    // TODO: would it be easier to just name the chords for each interval?
    // otherwise, I have to say "take the 2 from this scale, build a chord from it using a third up from _that_ note, and a fifth from _that_ note, even though those will be
    // major/minor and different "indicies" counting-wise

    fun buildChord(position: ScalePosition,
                   chordType: ChordType): Chord {

        return chordType.chordFromScale(position, this)
    }

    fun buildChordNotes(rootNote: Note,
                        position: ScalePosition,
                        chordType: ChordType): List<Note> {

        val intervals = chordType.intervalsFromScale(position, this)

        return intervals.map { interval ->
            interval.fromNote(rootNote)
        }
    }

    fun boundedIndex(index: Int) = index % intervals.size
}

interface ChordBuilder {
    fun fromRootNote(rootNote: Note): Array<Note>
}

class ChordType(val scalePositions: Array<Scale.ScalePosition>) {
    companion object {
        val TRIAD_CHORD = ChordType(arrayOf(
            Scale.ScalePosition.ROOT,
            Scale.ScalePosition.THIRD,
            Scale.ScalePosition.FIFTH
        ))
        val POWER_CHORD = ChordType(arrayOf(
            Scale.ScalePosition.ROOT,
            Scale.ScalePosition.FIFTH
        ))
        val SEVENTH_CHORD = ChordType(arrayOf(
            Scale.ScalePosition.ROOT,
            Scale.ScalePosition.THIRD,
            Scale.ScalePosition.FIFTH,
            Scale.ScalePosition.SEVENTH
        ))
        // TODO: How might a ChordType specify a non-Scale interval?
    }

    fun chordFromScale(position: Scale.ScalePosition, scale: Scale): Chord {
        val intervals = intervalsFromScale(position, scale)
        return Chord.findByIntervals(intervals)
    }

    fun intervalsFromScale(position: Scale.ScalePosition, scale: Scale): List<Interval> {
        return scalePositions.map { chordTypePositions ->
            val boundedIndex = scale.boundedIndex(chordTypePositions.scaleIntervalIndex + position.scaleIntervalIndex)
            scale.intervals[boundedIndex]
        }
        // TODO: looku p chord from set of intervals?

    }
}

class Chord(val name: String,
            val symbol: String,
            val intervals: List<Interval>) {

    companion object {
        fun findByIntervals(intervals: List<Interval>): Chord {
            val foundChord = ALL_CHORDS_I_CAN_THINK_OF_AT_THE_MOMENT.filter { chord ->
                chord.intervals.size == intervals.size && chord.intervals.containsAll(intervals)
            }.firstOrNull()

            return foundChord ?: Chord("I don't know", "", intervals);
        }

        val  MAJOR_TRIAD: Chord = Chord(
            "Major Triad", "", listOf(Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH))

        val  MAJOR_SUSPENDED_FOURTH_TRIAD: Chord = Chord(
            "Major Suspended Fourth Triad", "sus4", listOf(Interval.UNISON, Interval.FOURTH, Interval.FIFTH))

        val  MAJOR_SUSPENDED_SECOND_TRIAD: Chord = Chord(
            "Major Suspended Second Triad", "sus2", listOf(Interval.UNISON, Interval.MAJOR_SECOND, Interval.FIFTH))

        val  MINOR_TRIAD: Chord = Chord(
            "Minor Triad", "m", listOf(Interval.UNISON, Interval.MINOR_THIRD, Interval.FIFTH))

        val  DOMINANT_SEVENTH_CHORD: Chord = Chord(
            "Dominant 7th Chord", "7", listOf(Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH,
                Interval.MINOR_SEVENTH
            ))

        val  MAJOR_SEVENTH_CHORD: Chord = Chord(
            "Major 7th Chord", "M7",
            listOf(Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH, Interval.MAJOR_SEVENTH
            ))

        val  MINOR_SEVENTH_CHORD: Chord = Chord(
            "Minor 7th Chord", "m7",
            listOf(Interval.UNISON, Interval.MINOR_THIRD, Interval.FIFTH, Interval.MINOR_SEVENTH
            ))

        val  DIMINISHED_TRIAD: Chord = Chord(
            "Diminished Triad", "dim", listOf(Interval.UNISON, Interval.MINOR_THIRD, Interval.DIMINISHED_FIFTH))

        val  DIMINISHED_SEVENTH_CHORD: Chord = Chord(
            "Diminished 7th Chord", "dim7", listOf(Interval.UNISON,
                Interval.MINOR_THIRD, Interval.DIMINISHED_FIFTH, Interval.DIMINISHED_SEVENTH
            ))

        val  HALF_DIMINISHED_SEVENTH_CHORD: Chord = Chord(
            "Half-Diminished 7th Chord", "7b5", listOf(Interval.UNISON,
                Interval.MINOR_THIRD, Interval.DIMINISHED_FIFTH, Interval.MINOR_SEVENTH
            ))

        val ALL_CHORDS_I_CAN_THINK_OF_AT_THE_MOMENT = listOf(
            MAJOR_TRIAD,
            MAJOR_SUSPENDED_FOURTH_TRIAD,
            MAJOR_SUSPENDED_SECOND_TRIAD,
            MINOR_TRIAD,
            DOMINANT_SEVENTH_CHORD,
            MAJOR_SEVENTH_CHORD,
            MINOR_SEVENTH_CHORD,
            DIMINISHED_TRIAD,
            DIMINISHED_SEVENTH_CHORD,
            HALF_DIMINISHED_SEVENTH_CHORD
        )
    }

    fun buildNotes(rootNote: Note): List<Note> {
        return intervals.map { interval ->
            interval.fromNote(rootNote)
        }
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as Chord

        if (name != other.name) return false
        if (symbol != other.symbol) return false
        if (intervals != other.intervals) return false

        return true
    }

    override fun hashCode(): Int {
        var result = name.hashCode()
        result = 31 * result + symbol.hashCode()
        result = 31 * result + intervals.hashCode()
        return result
    }

    override fun toString(): String {
        return "Chord(name='$name', symbol='$symbol', intervals=$intervals)"
    }
}

class Progression(val scaleIndicies: List<Scale.ScalePosition>) {
    companion object {
        val I_iii_IV = Progression(
            listOf(Scale.ScalePosition.ROOT, Scale.ScalePosition.THIRD, Scale.ScalePosition.FOURTH)
        )

        val i_iii_IV_V = Progression(
            listOf(Scale.ScalePosition.ROOT, Scale.ScalePosition.THIRD, Scale.ScalePosition.FOURTH, Scale.ScalePosition.FIFTH)
        )
    }
}

class Key(val rootNote: Note, scale: Scale) {
}
