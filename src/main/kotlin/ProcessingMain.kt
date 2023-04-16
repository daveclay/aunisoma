import com.daveclay.aunisoma.audio.Audio
import com.daveclay.aunisoma.musictheory.Progression
import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.data.Buffer
import net.beadsproject.beads.ugens.Gain
import net.beadsproject.beads.ugens.Glide
import net.beadsproject.beads.ugens.ScalingMixer
import net.beadsproject.beads.ugens.WavePlayer
import processing.core.PApplet

fun main(args: Array<String>) {
    PApplet.main(MySketch::class.java, "")
}

class Voice(audioContext: AudioContext,
            mixer: ScalingMixer) {
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

    init {
        gain.addInput(wavePlayer)
        mixer.addInput(gain)
    }

    fun setFrequency(frequency: Float) {
        frequencyGlide.value = frequency
    }
}

class VoiceBuilder(private val audioContext: AudioContext,
                   private val mixer: ScalingMixer) {
    fun buildNewVoice() = Voice(audioContext, mixer)
}

class MySketch : PApplet() {
    private val audio = Audio()

    override fun settings() {
        size(500, 500)
    }

    override fun setup() {
        audio.initialize()
    }

    fun interpolateMouseWidth(mouseX: Float, size: Int): Int {
        return ((mouseX / width.toFloat()) * size).toInt()
    }

    override fun draw() {
        background(64)
        val mouseXf = mouseX.toFloat()
        val mouseYf = mouseY.toFloat()

        ellipse(mouseXf, mouseYf, 20f, 20f)

        val progression = Progression.i_iii_IV_V
        val scalePositionIndex = interpolateMouseWidth(mouseXf, progression.scalePositions.size)
        val scalePosition = progression.scalePositions[scalePositionIndex];

        audio.scalePosition = scalePosition
        val keyChord = audio.update()

        val notesText = "Notes: ${keyChord.notes.map { note -> note.name }.joinToString(", ")}"
        text("Chord: ${keyChord.name} - $notesText", 100f, 120f);
    }
}