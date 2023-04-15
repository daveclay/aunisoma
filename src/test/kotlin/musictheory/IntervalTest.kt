package musictheory

import org.junit.jupiter.api.Assertions.assertEquals

import org.junit.jupiter.api.Test

internal class IntervalTest {

    @Test
    fun testRootForRootInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, Scale.ScalePosition.ROOT, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.C, Note.E, Note.G), notes)
    }

    @Test
    fun testTriadChordForThirdInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, Scale.ScalePosition.THIRD, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.E, Note.G, Note.B), notes)
    }

    @Test
    fun testTriadChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, Scale.ScalePosition.FOURTH, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

    @Test
    fun testPowerChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, Scale.ScalePosition.FOURTH, ChordType.POWER_CHORD)
        assertEquals(listOf(Note.F, Note.C), notes)
    }

    @Test
    fun testBuildChordNotes() {
        val progression = Progression.I_iii_IV
        val scale = Scale.MAJOR_SCALE
        val rootNote = Note.C
        val index = 2 // root, third, fourth
        val scaleIndexToStart = progression.scaleIndicies[index]; // note to start with

        val notes = scale.buildChordNotes(rootNote, scaleIndexToStart, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

    @Test
    fun testFindChordByIntervalsMatchesRegardlessOfOrder() {
        val found = Chord.findByIntervals(listOf(Interval.UNISON, Interval.FIFTH, Interval.MAJOR_THIRD))
        assertEquals(Chord.MAJOR_TRIAD, found)
    }
}