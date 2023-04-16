package musictheory

/**
 * Is there a use for a Chord that references a Scale but _not_ a Key?
 */
class ScaleChord(val scale: Scale,
                 val intervalsRelativeToScale: List<Interval>) {

    fun name() = getChord().name

    fun getChord(): Chord {
        val shiftedIntervals = getShiftedDistancesForIntervals().map { distance ->
            Interval.intervalForDistance(distance)
        }
        return Chord.findByIntervals(shiftedIntervals)
    }

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

    fun buildNotes(key: Key): List<Note> {
        return intervalsRelativeToScale.map { interval ->
            interval.fromNote(key.rootNote)
        }
    }
}