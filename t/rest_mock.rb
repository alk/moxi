# This is a fake http/REST server.
#
#   ruby ./t/cluster_manager_mock.rb [optional_path_to_some_json_contents]
#
# For example...
#
#   ruby ./t/cluster_manager_mock.rb ./t/vbucket1.cfg
#
require 'rubygems'
gem 'sinatra'
gem 'rack-contrib'

require 'sinatra'
require 'rack/contrib'


class Application < Sinatra::Base
  get '/pools/default/buckets/default' do
    some_json
  end

  get '/pools/default/bucketsStreaming/default' do
    stream do
      some_json + "\n\n\n\n"
    end
  end

  get '/pools/default/bucketsStreamingConfig/default' do
    stream do
      some_json + "\n\n\n\n"
    end
  end

  class InfiniteStream
    def initialize(block)
      @block = block
    end
    def each
      while true
        rv = @block.call
        break unless rv
        yield rv
      end
    end
  end

  def stream(&block)
    first_time = true
    resp = InfiniteStream.new(lambda do
                                sleep 5 unless first_time
                                first_time = false
                                block.call
                              end)
    throw :response, [200, {'Content-Type' => 'application/json'}, resp]
  end

  def some_json()
    if ARGV.length > 0
      return File.read(ARGV[0])
    end

    <<-eos
{
  "name": "TheUser",
  "saslPassword": "ThePassword",
  "vBucketServerMap": {
    "hashAlgorithm": "CRC",
    "numReplicas": 0,
    "serverList": ["127.0.0.1:11311"],
    "vBucketMap":
      [
        [0]
      ]
  }
}
  eos
  end
end

$app = Rack::Builder.app do
  use Rack::CommonLogger
  use Rack::Evil
  run Application
end

config = {:app => $app, :Port => 4567}
handler = Rack::Handler.default(config)
handler.run $app, config
