package com.daveclay.aunisoma.musictheory

import com.daveclay.aunisoma.audio.Audio
import java.util.*
import kotlin.concurrent.thread


fun main(args: Array<String>) {
    thread(start = true) {
        CommandLineApp().start()
    }
}

class CommandLineApp {
    val audio = Audio()
    val reader = Scanner(System.`in`)
    fun start() {
        println("Type:")
        println("note=A")
        println("scale=maj")
        println("chord=1")
        println("chordType=TRIAD")

        val noteHandler: (String) -> Unit = { noteString ->
            val note = Note.findByString(noteString)
            audio.musicalKey = Key(note, audio.musicalKey.scale)
        }

        val scaleHandler: (String) -> Unit = { scaleString ->
            val scale = Scale.findByName(scaleString)
            if (scale != null) {
                audio.musicalKey = Key(audio.musicalKey.rootNote, scale)
            } else {
                val scales = Scale.values().map { scale -> scale.names }.flatten().joinToString(", ")
                println("Don't know what scale $scaleString is, try one of: $scales")
            }
        }

        val chordHandler: (String) -> Unit = { positionString ->
            val index = (Integer.valueOf(positionString) - 1) % Scale.ScalePosition.values().size
            val position = Scale.ScalePosition.values()[index]
            audio.scalePosition = position
        }

        val chordTypeHandler: (String) -> Unit = { chordTypeString ->
            val chordType = ChordType.findByName(chordTypeString)
            if (chordType != null) {
                audio.chordType = chordType
            } else {
                println("Don't know of a chord type of $chordTypeString - mostly because I'm probably confused about various chords vs the types of chords in a scale.")
            }
        }

        fun paramHandlerByName(name: String): (String) -> Unit {
            return when(name) {
                "note" -> noteHandler
                "scale" -> scaleHandler
                "chord" -> chordHandler
                "chordType" -> chordTypeHandler
                else -> {
                    throw IllegalArgumentException("Dunno what to do with $name - why don't you try again")
                }
            }
        }

        audio.initialize()
        while (true) {
            try {
                val input = reader.nextLine()

                val params = input.split("=")

                val paramName = params[0]
                val paramValue = params[1]
                val handler = paramHandlerByName(paramName)

                handler(paramValue)

                val keyChord = audio.update()

                val notesText = "Notes: ${keyChord.notes.map { note -> note.name }.joinToString(", ")}"
                println("Chord: ${keyChord.name} - $notesText")
            } catch (err: Exception) {
                err.printStackTrace()
            }
        }
    }
}