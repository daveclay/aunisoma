package musictheory

class Key(val rootNote: Note, val scale: Scale) {
    fun buildChord(position: Scale.ScalePosition, chordType: ChordType): Chord {
        val chord = scale.buildScaleChord(position, ChordType.TRIAD_CHORD)
        // TODO: shift chord intervals, name it, then return a ChordInKey that knows how to build its own notes and knows its name?

        return Chord.MAJOR_SEVENTH_CHORD
    }
}