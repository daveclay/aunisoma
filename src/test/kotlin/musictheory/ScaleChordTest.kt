package musictheory

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.Test

internal class ScaleChordTest {
    val scale = Scale.MAJOR_SCALE

    @Test
    fun testChordName_MinorTriadAtSecondPositionInCMajorScale() {
        val scaleChord = scale.buildScaleChord(Scale.ScalePosition.SECOND, ChordType.TRIAD_CHORD)
        assertEquals(Chord.MINOR_TRIAD, scaleChord.getChord())
    }

    @Test
    fun testChordName_MajorTriadAtFifthPositionInCMajorScale() {
        val scaleChord = scale.buildScaleChord(Scale.ScalePosition.FIFTH, ChordType.TRIAD_CHORD)
        assertEquals(listOf(Note.G, Note.B, Note.D), scaleChord.buildNotes(Note.C))
        assertEquals(Chord.MAJOR_TRIAD, scaleChord.getChord())
    }

    @Test
    fun testBuildChordNotes() {
        val scaleChord = scale.buildScaleChord(Scale.ScalePosition.FOURTH, ChordType.TRIAD_CHORD)
        val notes = scaleChord.buildNotes(Note.C)
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

}