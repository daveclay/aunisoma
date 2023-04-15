package musictheory

class Chord(val name: String,
            val symbol: String,
            val intervals: List<Interval>) {

    companion object {
        val  MAJOR_TRIAD: Chord = Chord(
            "Major Triad", "", listOf(Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH)
        )

        val  MAJOR_SUSPENDED_FOURTH_TRIAD: Chord = Chord(
            "Major Suspended Fourth Triad", "sus4", listOf(Interval.UNISON, Interval.FOURTH, Interval.FIFTH)
        )

        val  MAJOR_SUSPENDED_SECOND_TRIAD: Chord = Chord(
            "Major Suspended Second Triad", "sus2", listOf(Interval.UNISON, Interval.MAJOR_SECOND, Interval.FIFTH)
        )

        val  MINOR_TRIAD: Chord = Chord(
            "Minor Triad", "m", listOf(Interval.UNISON, Interval.MINOR_THIRD, Interval.FIFTH)
        )

        val  DOMINANT_SEVENTH_CHORD: Chord = Chord(
            "Dominant 7th Chord", "7", listOf(
                Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH,
                Interval.MINOR_SEVENTH
            )
        )

        val  MAJOR_SEVENTH_CHORD: Chord = Chord(
            "Major 7th Chord", "M7",
            listOf(
                Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH, Interval.MAJOR_SEVENTH
            )
        )

        val  MINOR_SEVENTH_CHORD: Chord = Chord(
            "Minor 7th Chord", "m7",
            listOf(
                Interval.UNISON, Interval.MINOR_THIRD, Interval.FIFTH, Interval.MINOR_SEVENTH
            )
        )

        val  DIMINISHED_TRIAD: Chord = Chord(
            "Diminished Triad", "dim", listOf(Interval.UNISON, Interval.MINOR_THIRD, Interval.DIMINISHED_FIFTH)
        )

        val  DIMINISHED_SEVENTH_CHORD: Chord = Chord(
            "Diminished 7th Chord", "dim7", listOf(
                Interval.UNISON,
                Interval.MINOR_THIRD, Interval.DIMINISHED_FIFTH, Interval.DIMINISHED_SEVENTH
            )
        )

        val  HALF_DIMINISHED_SEVENTH_CHORD: Chord = Chord(
            "Half-Diminished 7th Chord", "7b5", listOf(
                Interval.UNISON,
                Interval.MINOR_THIRD, Interval.DIMINISHED_FIFTH, Interval.MINOR_SEVENTH
            )
        )

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

        fun findByIntervals(intervals: List<Interval>): Chord {
            val foundChord = ALL_CHORDS_I_CAN_THINK_OF_AT_THE_MOMENT.filter { chord ->
                chord.intervals.size == intervals.size && chord.intervals.containsAll(intervals)
            }.firstOrNull()

            return foundChord ?: Chord("I don't know", "", intervals);
        }
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