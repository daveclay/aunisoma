package musictheory

class KeyChord(val scaleChord: ScaleChord, val key: Key) {

    fun getNotes(): List<Note> {
        return scaleChord.buildNotes(key)
    }

    fun name() = scaleChord.name()
}