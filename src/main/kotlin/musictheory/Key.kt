package musictheory

class Key(val rootNote: Note, val scale: Scale) {

    fun getChord(chordType: ChordType, scaleIndexToStart: Scale.ScalePosition): KeyChord {
        val scaleChord = scale.buildChordIntervals(chordType, scaleIndexToStart)
        return KeyChord(scaleChord, this)
    }
}