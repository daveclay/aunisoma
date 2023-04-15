package musictheory

import musictheory.Note.Companion.NOTES
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Assertions.assertEquals

import org.junit.jupiter.api.Test
import org.junit.jupiter.api.assertThrows
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.CsvSource

internal class IntervalTest {

    @Test
    fun testRootNotesForRootInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, 0, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.C, Note.E, Note.G), notes)
    }

    @Test
    fun testRootNotesForThirdInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, 2, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.E, Note.G, Note.B), notes)
    }

    @Test
    fun testRootNotesForFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, 3, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

    @Test
    fun hi() {
        val progression = Progression.I_iii_IV
        val scale = Scale.MAJOR_SCALE
        val rootNote = Note.C
        val index = 2 // root, third, fourth
        val scaleIndexToStart = progression.scaleIndicies[index]; // note to start with

        val notes = scale.buildChordNotes(rootNote, scaleIndexToStart, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }
}