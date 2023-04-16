package musictheory

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.Test

internal class ScaleChordTest {
    val scale = Scale.MAJOR_SCALE
    val key = Key(Note.C, Scale.MAJOR_SCALE)

    @Test
    fun testChordName_MinorTriadAtSecondPositionInCMajorScale() {
        val scaleChord = Scale.MAJOR_SCALE.buildScaleChord(ChordType.TRIAD_CHORD, Scale.ScalePosition.SECOND)
        assertEquals(Chord.MINOR_TRIAD, scaleChord.getChord())
    }

    @Test
    fun testChordName_MajorTriadAtFifthPositionInCMajorScale() {
        val scaleChord = Scale.MAJOR_SCALE.buildScaleChord(ChordType.TRIAD_CHORD, Scale.ScalePosition.FIFTH)
        assertEquals(listOf(Note.G, Note.B, Note.D), scaleChord.buildNotes(key))
        assertEquals(Chord.MAJOR_TRIAD, scaleChord.getChord())
    }

    @Test
    fun testBuildChordNotes() {
        val scaleChord = scale.buildScaleChord(ChordType.TRIAD_CHORD, Scale.ScalePosition.FOURTH)
        val notes = scaleChord.buildNotes(key)
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

}