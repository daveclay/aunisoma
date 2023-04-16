package musictheory

import org.junit.jupiter.api.Assertions.assertEquals

import org.junit.jupiter.api.Test

internal class IntervalTest {

    @Test
    fun testRootForRootInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chord = scale.buildScaleChord(ChordType.TRIAD_CHORD, Scale.ScalePosition.ROOT)
        val notes = chord.buildNotes(Key(Note.C, scale))
        assertEquals(listOf(Note.C, Note.E, Note.G), notes)
    }

    @Test
    fun testTriadChordForThirdInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chord = scale.buildScaleChord(ChordType.TRIAD_CHORD, Scale.ScalePosition.THIRD)
        val notes = chord.buildNotes(Key(Note.C, scale))
        assertEquals(listOf(Note.E, Note.G, Note.B), notes)
    }

    @Test
    fun testTriadChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chord = scale.buildScaleChord(ChordType.TRIAD_CHORD, Scale.ScalePosition.FOURTH)
        val notes = chord.buildNotes(Key(Note.C, scale))
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

    @Test
    fun testPowerChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chord = scale.buildScaleChord(ChordType.POWER_CHORD, Scale.ScalePosition.FOURTH)
        val notes = chord.buildNotes(Key(Note.C, scale))
        assertEquals(listOf(Note.F, Note.C), notes)
    }

    /*
    @Test
    fun testFindChordByIntervalsMatchesRegardlessOfOrder() {
        val found = Chord.findByIntervals(listOf(Interval.UNISON, Interval.FIFTH, Interval.MAJOR_THIRD))
        assertEquals(Chord.MAJOR_TRIAD, found)
    }
     */
}