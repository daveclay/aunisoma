package com.daveclay.aunisoma.musictheory

class Notes(val allNotes: Array<Note>) {
    val size = allNotes.size

    fun noteAt(index: Int): Note {
        val boundedIndex = index % size
        return allNotes[boundedIndex]
    }

    fun indexOf(note: Note): Int = allNotes.indexOf(note)
}