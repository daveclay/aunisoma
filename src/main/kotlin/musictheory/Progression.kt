package musictheory

class Progression(val scalePositions: List<Scale.ScalePosition>) {
    companion object {
        val I_iii_IV = Progression(
            listOf(Scale.ScalePosition.ROOT, Scale.ScalePosition.THIRD, Scale.ScalePosition.FOURTH)
        )

        val i_iii_IV_V = Progression(
            listOf(
                Scale.ScalePosition.ROOT,
                Scale.ScalePosition.THIRD,
                Scale.ScalePosition.FOURTH,
                Scale.ScalePosition.FIFTH
            )
        )
    }
}