package musictheory

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
        return Note.NOTES.noteAt(rootNote.indexOfNote() + index)
    }

    companion object {
        fun intervalForDistance(distance: Int): Interval {
            val boundDistance = distance % 12
            return values().first { interval -> interval.index == boundDistance }
        }
    }
}