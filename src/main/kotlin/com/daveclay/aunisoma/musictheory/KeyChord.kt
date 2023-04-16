package com.daveclay.aunisoma.musictheory

class KeyChord(private val chordIntervals: ChordIntervals, key: Key) {
    val name = chordIntervals.name
    val notes = buildNotes(key)

    private fun buildNotes(key: Key): List<Note> {
        return chordIntervals.intervalsRelativeToScale.map { interval ->
            interval.fromNote(key.rootNote)
        }
    }

    override fun toString(): String {
        val notesText = notes.map { note -> note.name }.joinToString(", ")
        return "$name ($notesText)"
    }
}