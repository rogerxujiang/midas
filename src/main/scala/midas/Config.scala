package midas

import core._
import widgets._
import platform._
import strober.core._
import freechips.rocketchip.config.{Parameters, Config, Field}
import freechips.rocketchip.amba.axi.AXI4BundleParameters

trait PlatformType
case object Zynq extends PlatformType
case object Catapult extends PlatformType
case object F1 extends PlatformType
case object Platform extends Field[PlatformType]
case object EnableSnapshot extends Field[Boolean]
case object MemModelKey extends Field[Option[Parameters => MemModel]]
case object EndpointKey extends Field[EndpointMap]

class SimConfig extends Config((site, here, up) => {
  case TraceMaxLen    => 1024
  case SRAMChainNum   => 1
  case ChannelLen     => 16
  case ChannelWidth   => 32
  case DaisyWidth     => 32
  case EnableSnapshot => false
  case CtrlAXIKey   => AXI4BundleParameters(dataBits = 32, addrBits = 32, idBits = 12)
  case MemAXIKey    => AXI4BundleParameters(dataBits = 64, addrBits = 32, idBits = 6)
  case EndpointKey    => EndpointMap(Seq(new SimNastiMemIO, new SimAXI4MemIO))
  case MemModelKey    => Some((p: Parameters) => new SimpleLatencyPipe()(p))
  case FpgaMMIOSize   => BigInt(1) << 12 // 4 KB
  case MidasLLCKey    => None
})

class ZynqConfig extends Config(new Config((site, here, up) => {
  case Platform       => Zynq
  case MasterAXIKey => site(CtrlAXIKey)
  case SlaveAXIKey  => site(MemAXIKey)
}) ++ new SimConfig)

class ZynqConfigWithSnapshot extends Config(new Config((site, here, up) => {
  case EnableSnapshot => true
}) ++ new ZynqConfig)

class CatapultConfig extends Config(new Config((site, here, up) => {
  case Platform       => Catapult
  case PCIeWidth      => 640
  case ChannelWidth   => 64
  case DaisyWidth     => 64
  case SoftRegKey     => SoftRegParam(32, 64)
  case CtrlAXIKey   => AXI4BundleParameters(dataBits = 64, addrBits = 32, idBits = 12)
  case AXIKey       => site(CtrlAXIKey)
  case SlaveAXIKey  => site(MemAXIKey)
}) ++ new SimConfig)

class CatapultConfigWithSnapshot extends Config(new Config((site, here, up) => {
  case EnableSnapshot => true
}) ++ new CatapultConfig)

class F1Config extends Config(new Config((site, here, up) => {
  case Platform       => F1
  case CtrlAXIKey   => AXI4BundleParameters(dataBits = 32, addrBits = 25, idBits = 12)
  case MemAXIKey    => AXI4BundleParameters(dataBits = 64, addrBits = 32, idBits = 16)
  case MasterAXIKey => site(CtrlAXIKey)
  case SlaveAXIKey => site(MemAXIKey)
}) ++ new SimConfig)

class F1ConfigWithSnapshot extends Config(new Config((site, here, up) => {
  case EnableSnapshot => true
}) ++ new F1Config)
