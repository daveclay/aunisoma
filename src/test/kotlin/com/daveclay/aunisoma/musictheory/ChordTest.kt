package com.daveclay.aunisoma.musictheory

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.Test

internal class ChordTest {

    @Test
    fun testChordName_MinorTriadAtSecondPositionInCMajorScale() {
        // C MAJOR_SCALE - SEVENTH - Chord: I don't know - Notes: B, D, F
        val chord = Chord.findByIntervals(
            listOf(
                Interval.UNISON,
                Interval.MAJOR_THIRD,
                Interval.DIMINISHED_FIFTH
            )
        )

        assertEquals(Chord.DIMINISHED, chord)
    }

}