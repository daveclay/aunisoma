package musictheory

/**
 * A Chord's Intervals are relative to _itself_, a ScaleChord's intervals are relative to the Scale.
 */
class ScaleChord(val positionInScale: Scale.ScalePosition,
                 val scale: Scale,
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

    // TODO: could just as well be Key instead of Note. It's the root of the key.
    fun buildNotes(rootNote: Note): Collection<Note> {
        return intervalsRelativeToScale.map { interval ->
            interval.fromNote(rootNote)
        }
    }
}