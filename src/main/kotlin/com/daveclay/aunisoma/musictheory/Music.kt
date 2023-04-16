package com.daveclay.aunisoma.musictheory

interface ChordBuilder {
    fun fromRootNote(rootNote: Note): Array<Note>
}

class ChordInKey(val chord: Chord, val key: Key) {
}

