package com.daveclay.aunisoma.musictheory

import com.daveclay.aunisoma.musictheory.*
import org.junit.jupiter.api.Assertions.assertEquals

import org.junit.jupiter.api.Test

internal class KeyChordTest {
    /**
     * C MINOR_SCALE - ROOT - Chord: Minor Triad - Notes: C, D#, G
    C MAJOR_SCALE - THIRD - Chord: Major Triad - Notes: D#, G, A#
    C MAJOR_SCALE - SECOND - Chord: I don't know - Notes: D, F, G#
     */
    @Test
    fun testBug() {
        val scale = Scale.MAJOR_SCALE

    }

    @Test
    fun testRootForRootInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.ROOT)
        val keyChord = KeyChord(chordIntervals, Key(Note.C, scale))
        val notes = keyChord.notes
        assertEquals(listOf(Note.C, Note.E, Note.G), notes)
    }

    @Test
    fun testTriadChordForSecondInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.SECOND)
        val keyChord = KeyChord(chordIntervals, Key(Note.C, scale))
        val notes = keyChord.notes
        assertEquals(listOf(Note.D, Note.F, Note.A), notes)
    }

    @Test
    fun testTriadChordForThirdInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.THIRD)
        val keyChord = KeyChord(chordIntervals, Key(Note.C, scale))
        val notes = keyChord.notes
        assertEquals(listOf(Note.E, Note.G, Note.B), notes)
    }

    @Test
    fun testTriadChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.FOURTH)
        val keyChord = KeyChord(chordIntervals, Key(Note.C, scale))
        val notes = keyChord.notes
        assertEquals(listOf(Note.F, Note.A, Note.C), notes)
    }

    @Test
    fun testPowerChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.POWER_CHORD, Scale.ScalePosition.FOURTH)
        val keyChord = KeyChord(chordIntervals, Key(Note.C, scale))
        val notes = keyChord.notes
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