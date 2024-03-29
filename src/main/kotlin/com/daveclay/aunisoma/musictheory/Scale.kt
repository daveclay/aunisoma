package com.daveclay.aunisoma.musictheory

import kotlin.random.Random

enum class Scale(val names: List<String>, vararg val intervals: Interval) {
    UNISON_SCALE(
        listOf("Unison"), Interval.UNISON),
    PERFECT_SCALE( listOf("Perfect"), Interval.UNISON, Interval.FOURTH, Interval.FIFTH),
    MINOR_PENTATONIC_SCALE( listOf("Minor Pentatonic"),
        Interval.UNISON,
        Interval.MINOR_THIRD,
        Interval.FOURTH,
        Interval.FIFTH,
        Interval.MINOR_SEVENTH
    ),
    MAJOR_PENTATONIC_SCALE( listOf("Major Pentatonic"),
        Interval.UNISON,
        Interval.MAJOR_THIRD,
        Interval.FOURTH,
        Interval.FIFTH,
        Interval.MAJOR_SEVENTH
    ),
    BLUES_CHROMATIC_SCALE( listOf("Blues Chromatic"),
        Interval.UNISON,
        Interval.MINOR_THIRD,
        Interval.MAJOR_THIRD,
        Interval.FOURTH,
        Interval.FLAT_FIFTH,
        Interval.FIFTH,
        Interval.MINOR_SEVENTH
    ),
    BLUES_MAJOR_SCALE( listOf("Blues Major"),
        Interval.UNISON,
        Interval.MAJOR_THIRD,
        Interval.FOURTH,
        Interval.FLAT_FIFTH,
        Interval.FIFTH,
        Interval.MAJOR_SEVENTH
    ),
    BLUES_MINOR_SCALE( listOf("Blues Minor"),
        Interval.UNISON,
        Interval.MINOR_THIRD,
        Interval.FOURTH,
        Interval.FLAT_FIFTH,
        Interval.FIFTH,
        Interval.MINOR_SEVENTH
    ),
    MINOR_SCALE( listOf("Minor", "min"),
        Interval.UNISON,
        Interval.MAJOR_SECOND,
        Interval.MINOR_THIRD,
        Interval.FOURTH,
        Interval.FIFTH,
        Interval.MINOR_SIXTH,
        Interval.MINOR_SEVENTH
    ),
    MAJOR_SCALE( listOf("Major", "maj"),
        Interval.UNISON,
        Interval.MAJOR_SECOND,
        Interval.MAJOR_THIRD,
        Interval.FOURTH,
        Interval.FIFTH,
        Interval.MAJOR_SIXTH,
        Interval.MAJOR_SEVENTH
    );

    enum class ScalePosition(val scaleIntervalIndex: Int) {
        // Which Interval in the Scale?
        ROOT(0),
        SECOND(1),
        THIRD(2),
        FOURTH(3),
        FIFTH(4),
        SIXTH(5),
        SEVENTH(6);
    }

    // TODO: would it be easier to just name the chords for each interval?
    // otherwise, I have to say "take the 2 from this scale, build a chord from it using a third up from _that_ note, and a fifth from _that_ note, even though those will be
    // major/minor and different "indicies" counting-wise

    fun randomPosition(): ScalePosition {
        return ScalePosition.values()[Random.nextInt(ScalePosition.values().size)]
    }

    fun buildChordIntervals(chordType: ChordType, position: ScalePosition): ChordIntervals {
        val intervals = intervalsForChordTypeAtPosition(chordType, position)
        return ChordIntervals(intervals)
    }

    private fun intervalsForChordTypeAtPosition(chordType: ChordType, position: ScalePosition): List<Interval> {
        return chordType.scalePositions.map { chordTypePositions ->
            val boundedIndex = boundedIndex(chordTypePositions.scaleIntervalIndex + position.scaleIntervalIndex)
            intervals[boundedIndex]
        }
    }

    private fun boundedIndex(index: Int) = index % intervals.size

    companion object {
        fun findByName(scaleString: String): Scale? {
            return values().firstOrNull { scale ->
                scale.name == scaleString || scale.names.contains(scaleString)
            }
        }
    }
}