package musictheory

/**
 * A ChordInterval represents a Chord built from intervals relative to a Scale. By referencing the Intervals relative
 * to the root Interval, the Chord can be inferred. This class is used to build up a KeyChord
 */
class ChordIntervals(val intervalsRelativeToScale: List<Interval>) {

    // ChordIntervals represent
    private val intervalsRelativeToRoot = getShiftedDistancesForIntervals().map { distance ->
        Interval.intervalForDistance(distance)
    }

    val chord = Chord.findByIntervals(intervalsRelativeToRoot)
    val name = chord.name

    private fun getShiftedDistancesForIntervals(): List<Int> {
        val rootInterval = intervalsRelativeToScale.get(0)
        return intervalsRelativeToScale.map { interval ->
            getDistance(rootInterval, interval)
        }
    }

    private fun getDistance(rootInterval: Interval, interval: Interval): Int {
        if (interval.index > rootInterval.index) {
            return interval.index - rootInterval.index
        } else {
            return (interval.index + 12) - rootInterval.index
        }
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as ChordIntervals

        if (intervalsRelativeToScale != other.intervalsRelativeToScale) return false

        return true
    }

    override fun hashCode(): Int {
        return intervalsRelativeToScale.hashCode()
    }

    override fun toString(): String {
        return "ChordIntervals(intervalsRelativeToScale=$intervalsRelativeToScale, name='$name')"
    }
}