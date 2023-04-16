package com.daveclay.aunisoma.musictheory

class Chord(val name: String,
            val symbol: String,
            val intervals: List<Interval>) {

    companion object {
        val  MAJOR_TRIAD = Chord(
            "Major Triad", "M",
            listOf(
                Interval.UNISON,
                Interval.MAJOR_THIRD,
                Interval.FIFTH
            )
        )

        val  MAJOR_SUSPENDED_FOURTH_TRIAD = Chord(
            "Major Suspended Fourth Triad", "sus4",
            listOf(
                Interval.UNISON,
                Interval.FOURTH,
                Interval.FIFTH
            )
        )

        val  MAJOR_SUSPENDED_SECOND_TRIAD = Chord(
            "Major Suspended Second Triad", "sus2",
            listOf(
                Interval.UNISON,
                Interval.MAJOR_SECOND,
                Interval.FIFTH
            )
        )

        val  MINOR_TRIAD = Chord(
            "Minor Triad", "m",
            listOf(
                Interval.UNISON,
                Interval.MINOR_THIRD,
                Interval.FIFTH
            )
        )

        val  DIMINISHED = Chord(
            "Diminished", "dim",
            listOf(
                Interval.UNISON,
                Interval.MAJOR_THIRD,
                Interval.DIMINISHED_FIFTH
            )
        )

        val  DOMINANT_SEVENTH_CHORD = Chord(
            "Dominant 7th ", "7",
            listOf(
                Interval.UNISON,
                Interval.MAJOR_THIRD,
                Interval.FIFTH,
                Interval.MINOR_SEVENTH
            )
        )

        val  MAJOR_SEVENTH_CHORD = Chord(
            "Major 7th ", "M7",
            listOf(
                Interval.UNISON,
                Interval.MAJOR_THIRD,
                Interval.FIFTH,
                Interval.MAJOR_SEVENTH
            )
        )

        val  MINOR_SEVENTH_CHORD = Chord(
            "Minor 7th ", "m7",
            listOf(
                Interval.UNISON,
                Interval.MINOR_THIRD,
                Interval.FIFTH,
                Interval.MINOR_SEVENTH
            )
        )

        val  DIMINISHED_SEVENTH_CHORD = Chord(
            "Diminished 7th ", "dim7", listOf(
                Interval.UNISON,
                Interval.MINOR_THIRD,
                Interval.DIMINISHED_FIFTH,
                Interval.DIMINISHED_SEVENTH
            )
        )

        val  HALF_DIMINISHED_SEVENTH_CHORD = Chord(
            "Half-Diminished 7th", "7b5", listOf(
                Interval.UNISON,
                Interval.MINOR_THIRD,
                Interval.DIMINISHED_FIFTH,
                Interval.MINOR_SEVENTH
            )
        )

        val FIFTH = Chord(
            "Fifth", "5", listOf(
                Interval.UNISON,
                Interval.FIFTH
            )
        )

        val ALL_CHORDS_I_CAN_THINK_OF_AT_THE_MOMENT = listOf(
            MAJOR_TRIAD,
            MAJOR_SUSPENDED_FOURTH_TRIAD,
            MAJOR_SUSPENDED_SECOND_TRIAD,
            MINOR_TRIAD,
            DIMINISHED,
            FIFTH,
            DOMINANT_SEVENTH_CHORD,
            MAJOR_SEVENTH_CHORD,
            MINOR_SEVENTH_CHORD,
            DIMINISHED_SEVENTH_CHORD,
            HALF_DIMINISHED_SEVENTH_CHORD
        )

        fun findByName(name: String): Chord? {
            return ALL_CHORDS_I_CAN_THINK_OF_AT_THE_MOMENT.first { chord ->
                chord.name == name || chord.symbol == name
            }
        }

        fun findByIntervals(intervals: List<Interval>): Chord {
            // TODO: find matching indexes if not found (DIMINISHED vs FLAT_FIFTH, etc)
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