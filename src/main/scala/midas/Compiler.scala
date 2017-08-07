package midas

import chisel3.Data
import firrtl._
import firrtl.ir.Circuit
import firrtl.CompilerUtils.getLoweringTransforms
import firrtl.passes.memlib._
import barstools.macros._
import java.io.{File, FileWriter, Writer}

// Compiler for Midas Transforms
private class MidasCompiler(dir: File, io: Data)(implicit param: config.Parameters) extends Compiler {
  def emitter = new LowFirrtlEmitter
  def transforms =
    getLoweringTransforms(ChirrtlForm, MidForm) ++ Seq(
    new InferReadWrite,
    new ReplSeqMem) ++
    getLoweringTransforms(MidForm, LowForm) ++ Seq(
    new passes.MidasTransforms(dir, io))
}

// Compilers to emit proper verilog
private class VerilogCompiler extends Compiler {
  def emitter = new VerilogEmitter
  def transforms = getLoweringTransforms(HighForm, LowForm) :+ (
    new LowFirrtlOptimization)
}

object MidasCompiler {
  def apply(
      chirrtl: Circuit,
      io: Data,
      dir: File,
      lib: Option[File])
     (implicit p: config.Parameters): Circuit = {
    val conf = new File(dir, s"${chirrtl.main}.conf")
    val json = new File(dir, s"${chirrtl.main}.macros.json")
    val annotations = new AnnotationMap(Seq(
      InferReadWriteAnnotation(chirrtl.main),
      ReplSeqMemAnnotation(s"-c:${chirrtl.main}:-o:$conf"),
      passes.MidasAnnotation(chirrtl.main, conf, json, lib),
      MacroCompilerAnnotation(chirrtl.main, MacroCompilerAnnotation.Params(
        json.toString, lib map (_.toString), CostMetric.default, true))))
    val midas = new MidasCompiler(dir, io) compileAndEmit (
      CircuitState(chirrtl, ChirrtlForm, Some(annotations)))
    val result = new VerilogCompiler compileAndEmit (
      CircuitState(midas.circuit, HighForm, Some(
      new AnnotationMap(Seq(transforms.DontCheckCombLoopsAnnotation())))))
    val verilog = new FileWriter(new File(dir, s"FPGATop.v"))
    verilog write result.getEmittedCircuit.value
    verilog.close
    result.circuit
  }

  def apply[T <: chisel3.Module](
      w: => T,
      dir: File,
      lib: Option[File] = None)
     (implicit p: config.Parameters): Circuit = {
    dir.mkdirs
    lazy val target = w
    val chirrtl = Parser.parse(chisel3.Driver.emit(() => target))
    apply(chirrtl, target.io, dir, lib)
  }
}
