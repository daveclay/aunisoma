package com.daveclay.aunisoma.musictheory

import com.daveclay.aunisoma.musictheory.ChordIntervals
import com.daveclay.aunisoma.musictheory.ChordType
import com.daveclay.aunisoma.musictheory.Interval
import com.daveclay.aunisoma.musictheory.Scale
import org.junit.jupiter.api.Assertions.assertEquals

import org.junit.jupiter.api.Test

internal class ScaleTest {

    @Test
    fun testRootForRootInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.ROOT)
        assertEquals(ChordIntervals(listOf(Interval.UNISON, Interval.MAJOR_THIRD, Interval.FIFTH)), chordIntervals)
    }

    @Test
    fun testTriadChordForThirdInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.THIRD)
        assertEquals(
            ChordIntervals(listOf(Interval.MAJOR_THIRD, Interval.FIFTH, Interval.MAJOR_SEVENTH)), chordIntervals)
    }

    @Test
    fun testTriadChordAtSecondInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.SECOND)
        assertEquals(
            ChordIntervals(listOf(Interval.MAJOR_SECOND, Interval.FOURTH, Interval.MAJOR_SIXTH)), chordIntervals)
    }

    @Test
    fun testTriadChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.TRIAD_CHORD, Scale.ScalePosition.FOURTH)
        assertEquals(
            ChordIntervals(listOf(Interval.FOURTH, Interval.MAJOR_SIXTH, Interval.UNISON)), chordIntervals)
    }

    @Test
    fun testPowerChordAtFourthInMajorScale() {
        val scale = Scale.MAJOR_SCALE
        val chordIntervals = scale.buildChordIntervals(ChordType.POWER_CHORD, Scale.ScalePosition.FOURTH)
        assertEquals(
            ChordIntervals(listOf(Interval.FOURTH, Interval.UNISON)), chordIntervals)
    }

    /*
    @Test
    fun testFindChordByIntervalsMatchesRegardlessOfOrder() {
        val found = Chord.findByIntervals(listOf(Interval.UNISON, Interval.FIFTH, Interval.MAJOR_THIRD))
        assertEquals(Chord.MAJOR_TRIAD, found)
    }
     */
}