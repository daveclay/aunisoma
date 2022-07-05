import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.data.Buffer
import net.beadsproject.beads.ugens.Gain
import net.beadsproject.beads.ugens.Glide
import net.beadsproject.beads.ugens.WavePlayer
import processing.core.PApplet

class NoteFrequencies {
    companion object {
        val C = arrayOf(
            65.41f, 130.81f, 261.63f, 523.25f, 1046.50f, 2093.00f, 4186.01f)
        val C_SHARP = arrayOf(
            69.30f, 138.59f, 277.18f, 554.37f, 1108.73f, 2217.46f, 4434.92f)
        val D = arrayOf(
            73.42f, 146.83f, 293.66f, 587.33f, 1174.66f, 2349.32f, 4698.63f)
        val D_SHARP = arrayOf(
            77.78f, 155.56f, 311.13f, 622.25f, 1244.51f, 2489.02f, 4978.03f)
        val E = arrayOf(
            82.41f, 164.81f, 329.63f, 659.25f, 1318.51f, 2637.02f, 5274.04f)
        val F = arrayOf(
            87.31f, 174.61f, 349.23f, 698.46f, 1396.91f, 2793.83f, 5587.65f)
        val F_SHARP = arrayOf(
            92.50f, 185.00f, 369.99f, 739.99f, 1479.98f, 2959.96f, 5919.91f)
        val G = arrayOf(
            98.00f, 196.00f, 392.00f, 783.99f, 1567.98f, 3135.96f, 6271.93f)
        val G_SHARP = arrayOf(
            103.83f, 207.65f, 415.30f, 830.61f, 1661.22f, 3322.44f, 6644.88f)
        val A = arrayOf(
            110.00f, 220.00f, 440.00f, 880.00f, 1760.00f, 3520.00f, 7040.00f)
        val A_SHARP = arrayOf(
            116.54f, 233.08f, 466.16f, 932.33f, 1864.66f, 3729.31f, 7458.62f)
        val B = arrayOf(
            123.47f, 246.94f, 493.88f, 987.77f, 1975.53f, 3951.07f, 7902.13f)

        val NOTES = arrayOf(
            C,
            C_SHARP,
            D,
            D_SHARP,
            E,
            F,
            F_SHARP,
            G,
            G_SHARP,
            A,
            A_SHARP,
            B
        )
    }
}

/**
 * https://manualzz.com/doc/23754747/sonifying-processing--the-beads-tutorial
 */
class Audio {
    private val audioContext = AudioContext()

    private val frequencyGlide = Glide(audioContext,
        440f,
        0f);

    private val wavePlayer = WavePlayer(
        audioContext,
        frequencyGlide,
        Buffer.SINE)

    private val gainGlide = Glide(
        audioContext,
        0.5f,
        0f);

    private val gain = Gain(
        audioContext,
        1,
        gainGlide
    )

    fun initialize() {
        gain.addInput(wavePlayer)
        audioContext.out.addInput(gain)
        audioContext.start()
    }

    fun setGain(gain: Float) {
        gainGlide.value = gain
    }

    fun setFrequency(freq: Float) {
        frequencyGlide.value = freq
    }
}

class MySketch : PApplet() {
    private val audio = Audio()

    override fun settings() {
        size(500, 500)
    }

    override fun setup() {
        audio.initialize()
    }

    override fun draw() {
        background(64)
        val mouseXf = mouseX.toFloat()
        val mouseYf = mouseY.toFloat()
        val heightf = height.toFloat()
        val noteIndex = ((mouseXf / width.toFloat()) * NoteFrequencies.NOTES.size).toInt()
        val note = NoteFrequencies.NOTES.get(noteIndex)
        val octaveIndex = ((mouseYf / heightf) * note.size).toInt()
        val freq = note.get(octaveIndex)

        ellipse(mouseXf, mouseYf, 20f, 20f)
        audio.setFrequency(freq)

        text("Freq: $freq", 100f, 120f);
    }
}

fun main(args: Array<String>) {
    PApplet.main(MySketch::class.java, "")
}