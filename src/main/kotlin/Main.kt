import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.data.Buffer
import net.beadsproject.beads.ugens.Gain
import net.beadsproject.beads.ugens.Glide
import net.beadsproject.beads.ugens.WavePlayer
import processing.core.PApplet

/**
 * https://manualzz.com/doc/23754747/sonifying-processing--the-beads-tutorial
 */
class Audio {
    private val audioContext = AudioContext()

    private val frequencyGlide = Glide(audioContext,
        440f,
        10f);

    private val wavePlayer = WavePlayer(
        audioContext,
        frequencyGlide,
        Buffer.SINE)

    private val gainGlide = Glide(
        audioContext,
        0.3f,
        10f);

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
        val gain = mouseXf / width.toFloat()
        val freq = (mouseYf / heightf) * 1000

        ellipse(mouseXf, mouseYf, 20f, 20f)
        audio.setGain(gain)
        audio.setFrequency(freq)

        text("Gain: $gain", 100f, 100f);
        text("Freq: $freq", 100f, 120f);
    }
}

fun main(args: Array<String>) {
    PApplet.main(MySketch::class.java, "")
}