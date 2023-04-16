package com.daveclay.aunisoma.audio

import com.daveclay.aunisoma.musictheory.Interval
import com.daveclay.aunisoma.musictheory.Key
import com.daveclay.aunisoma.musictheory.Note
import com.daveclay.aunisoma.musictheory.Scale
import java.util.Timer
import java.util.TimerTask
import kotlin.random.Random

fun main(args: Array<String>) {
    Timer().scheduleAtFixedRate(Go(), 0, 2000)
}

class Go : TimerTask() {
    val audio = Audio()
    init {
        audio.initialize()
    }

    override fun run() {
        try {
            val keyChord = audio.update()
            println("${audio.key.rootNote} ${audio.key.scale.name} - ${audio.scalePosition.name} - $keyChord")

            audio.scalePosition = audio.key.scale.randomPosition()
            if (audio.key.scale == Scale.MINOR_SCALE) {
                if (audio.scalePosition == Scale.ScalePosition.THIRD) {
                    val relativeMajor = Interval.MINOR_THIRD.fromNote(audio.key.rootNote)
                    audio.key = Key(relativeMajor, Scale.MAJOR_SCALE)
                }
            } else {
                if (audio.scalePosition == Scale.ScalePosition.THIRD) {
                    audio.key = Key(audio.key.rootNote, Scale.MINOR_SCALE)
                }
            }


        } catch (err: Exception) {
            err.printStackTrace()
        }
    }
}