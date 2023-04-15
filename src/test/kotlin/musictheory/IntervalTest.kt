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
        val notes = scale.buildChordNotes(Note.C, 0)
        assertEquals(Note.C, notes[0])
        assertEquals(Note.E, notes[1])
        assertEquals(Note.G, notes[2])
    }

    @Test
    fun testRootNotesForThirdInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, 2)
        assertEquals(Note.E, notes[0])
        assertEquals(Note.G, notes[1])
        assertEquals(Note.B, notes[2])
    }

    @Test
    fun testRootNotesForFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val notes = scale.buildChordNotes(Note.C, 3)
        assertEquals(Note.F, notes[0])
        assertEquals(Note.A, notes[1])
        assertEquals(Note.C, notes[2])
    }

    @Test
    fun hi() {
        val progression = Progression.I_iii_IV
        val scale = Scale.MAJOR_SCALE
        val rootNote = Note.C
        val index = 2 // root, third, fourth
        val scaleIndexToStart = progression.scaleIndicies[index]; // note to start with

        val notes = scale.buildChordNotes(rootNote, scaleIndexToStart)
        assertEquals(Note.F, notes[0])
        assertEquals(Note.A, notes[1])
        assertEquals(Note.C, notes[2])

    }
}