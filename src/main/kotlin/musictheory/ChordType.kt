package musictheory

class ChordType(val scalePositions: Array<Scale.ScalePosition>) {
    companion object {
        val TRIAD_CHORD = ChordType(
            arrayOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.THIRD,
                Scale.ScalePosition.FIFTH
            )
        )
        val POWER_CHORD = ChordType(
            arrayOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.FIFTH
            )
        )
        val SEVENTH_CHORD = ChordType(
            arrayOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.THIRD,
                Scale.ScalePosition.FIFTH,
                Scale.ScalePosition.SEVENTH
            )
        )
        // TODO: How might a ChordType specify a non-Scale interval?
    }
}