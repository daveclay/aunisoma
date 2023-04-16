package musictheory

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.Test

internal class ChordIntervalsTest {
    val scale = Scale.MAJOR_SCALE
    val key = Key(Note.C, Scale.MAJOR_SCALE)

    @Test
    fun testChordName_MinorTriadAtSecondPositionInCMajorScale() {
        val chordIntervals = Scale.MAJOR_SCALE.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.SECOND)
        assertEquals(Chord.MINOR_TRIAD, chordIntervals.chord)
    }

    @Test
    fun testChordName_MajorTriadAtFifthPositionInCMajorScale() {
        val scaleChord = Scale.MAJOR_SCALE.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.FIFTH)
        assertEquals(Chord.MAJOR_TRIAD, scaleChord.chord)
    }

    @Test
    fun testBuildChordNotes() {
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.FOURTH)
        val keyChord = KeyChord(chordIntervals, Key(Note.C, Scale.MAJOR_SCALE))
        val notes = keyChord.notes
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

}