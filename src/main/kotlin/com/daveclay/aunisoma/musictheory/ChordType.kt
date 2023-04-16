package com.daveclay.aunisoma.musictheory

class ChordType(val names: List<String>,
                val scalePositions: Array<Scale.ScalePosition>) {
    companion object {
        fun findByName(chordTypeString: String): ChordType? {
            return ALL_TYPES.first { chordType ->
                chordType.names.contains(chordTypeString)
            }
        }

        val TRIAD_CHORD = ChordType(
            listOf(
                "triad"
            ),
            arrayOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.THIRD,
                Scale.ScalePosition.FIFTH
            )
        )
        val POWER_CHORD = ChordType(
            listOf(
                "5",
                "5th",
                "power",
                "fifth"
            ),
            arrayOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.FIFTH
            )
        )
        val SEVENTH_CHORD = ChordType(
            listOf(
                "seventh",
                "7",
                "7th"
            ),
            arrayOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.THIRD,
                Scale.ScalePosition.FIFTH,
                Scale.ScalePosition.SEVENTH
            )
        )

        val ALL_TYPES = listOf(
            TRIAD_CHORD,
            POWER_CHORD,
            SEVENTH_CHORD
        )
        // TODO: How might a ChordType specify a non-Scale interval?
    }
}