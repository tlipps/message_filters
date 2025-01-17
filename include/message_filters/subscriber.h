/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2009, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

#ifndef MESSAGE_FILTERS__SUBSCRIBER_H_
#define MESSAGE_FILTERS__SUBSCRIBER_H_

#include <stdexcept>
#include <type_traits>

#include <rclcpp/rclcpp.hpp>

#include "message_filters/connection.h"
#include "message_filters/simple_filter.h"

namespace message_filters
{

template<class NodeType = rclcpp::Node>
class SubscriberBase
{
public:
  typedef std::shared_ptr<NodeType> NodePtr;

  virtual ~SubscriberBase() = default;
  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node::SharedPtr to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe
   */
  virtual void subscribe(NodePtr node, const std::string& topic, const rmw_qos_profile_t qos = rmw_qos_profile_default) = 0;

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe
   */
  virtual void subscribe(NodeType * node, const std::string& topic, const rmw_qos_profile_t qos = rmw_qos_profile_default) = 0;

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   * This override allows SubscriptionOptions to be passed into the class without changing API.
   *
   * \param node The rclcpp::Node::SharedPtr to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe.
   * \param options The subscription options to use to subscribe.
   */
  virtual void subscribe(
    NodePtr node,
    const std::string& topic,
    const rmw_qos_profile_t qos,
    rclcpp::SubscriptionOptions options)
  {
    this->subscribe(node.get(), topic, qos, options);
  };

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe
   */
  virtual void subscribe(
    NodeType * node,
    const std::string& topic,
    const rmw_qos_profile_t qos,
    rclcpp::SubscriptionOptions options)
  {
    this->subscribe(node, topic, qos, options);
  }

  /**
   * \brief Re-subscribe to a topic.  Only works if this subscriber has previously been subscribed to a topic.
   */
  virtual void subscribe() = 0;
  /**
   * \brief Force immediate unsubscription of this subscriber from its topic
   */
  virtual void unsubscribe() = 0;
};
template <typename T>
using SubscriberBasePtr = std::shared_ptr<SubscriberBase<T>>;

/**
 * \brief ROS subscription filter.
 *
 * This class acts as a highest-level filter, simply passing messages from a ROS subscription through to the
 * filters which have connected to it.
 *
 * When this object is destroyed it will unsubscribe from the ROS subscription.
 *
 * The Subscriber object is templated on the type of message being subscribed to.
 *
 * \section connections CONNECTIONS
 *
 * Subscriber has no input connection.
 *
 * The output connection for the Subscriber object is the same signature as for rclcpp subscription callbacks, ie.
\verbatim
void callback(const std::shared_ptr<M const>&);
\endverbatim
 */

template <typename M, bool is_adapter = rclcpp::is_type_adapter<M>::value>
struct message_type;

template <typename M>
struct message_type <M, true>
{
  using type = typename M::custom_type;
};

template <typename M> 
struct message_type <M, false> 
{
  using type = M;
};

template <typename M>
using message_type_t = typename message_type<M>::type;

template<class M, class NodeType = rclcpp::Node>
class Subscriber 
: public SubscriberBase<NodeType>
, public SimpleFilter<message_type_t<M>>
{
public:
  typedef std::shared_ptr<NodeType> NodePtr;
  typedef message_type_t<M> MessageType;
  typedef MessageEvent<MessageType const> EventType;

  /**
   * \brief Constructor
   *
   * See the rclcpp::Node::subscribe() variants for more information on the parameters
   *
   * \param node The rclcpp::Node::SharedPtr to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe
   */
  Subscriber(NodePtr node, const std::string& topic, const rmw_qos_profile_t qos = rmw_qos_profile_default)
  {
    subscribe(node, topic, qos);
  }

  Subscriber(NodeType * node, const std::string& topic, const rmw_qos_profile_t qos = rmw_qos_profile_default)
  {
    subscribe(node, topic, qos);
  }

  /**
   * \brief Constructor
   *
   * See the rclcpp::Node::subscribe() variants for more information on the parameters
   *
   * \param node The rclcpp::Node::SharedPtr to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos The rmw qos profile to use to subscribe.
   * \param options The subscription options to use to subscribe.
   */
  Subscriber(
    NodePtr node,
    const std::string& topic,
    const rmw_qos_profile_t qos,
    rclcpp::SubscriptionOptions options)
  {
      subscribe(node.get(), topic, qos, options);
  }

  Subscriber(
    NodeType * node,
    const std::string& topic,
    const rmw_qos_profile_t qos,
    rclcpp::SubscriptionOptions options)
  {
    subscribe(node, topic, qos, options);
  }

  /**
   * \brief Empty constructor, use subscribe() to subscribe to a topic
   */
  Subscriber() = default;

  ~Subscriber()
  {
    unsubscribe();
  }

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node::SharedPtr to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe
   */
  void subscribe(NodePtr node, const std::string& topic, const rmw_qos_profile_t qos = rmw_qos_profile_default) override
  {
    subscribe(node.get(), topic, qos, rclcpp::SubscriptionOptions());
  }

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos (optional) The rmw qos profile to use to subscribe
   */
  // TODO(wjwwood): deprecate in favor of API's that use `rclcpp::QoS` instead.
  void subscribe(NodeType * node, const std::string& topic, const rmw_qos_profile_t qos = rmw_qos_profile_default) override
  {
    subscribe(node, topic, qos, rclcpp::SubscriptionOptions());
  }

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node::SharedPtr to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos The rmw qos profile to use to subscribe.
   * \param options The subscription options to use to subscribe.
   */
  void subscribe(
    NodePtr node,
    const std::string& topic,
    const rmw_qos_profile_t qos,
    rclcpp::SubscriptionOptions options) override
  {
    subscribe(node.get(), topic, qos, options);
    node_raw_ = nullptr;
    node_shared_ = node;
  }

  /**
   * \brief Subscribe to a topic.
   *
   * If this Subscriber is already subscribed to a topic, this function will first unsubscribe.
   *
   * \param node The rclcpp::Node to use to subscribe.
   * \param topic The topic to subscribe to.
   * \param qos The rmw qos profile to use to subscribe
   * \param options The subscription options to use to subscribe.
   */
  // TODO(wjwwood): deprecate in favor of API's that use `rclcpp::QoS` instead.
  void subscribe(
    NodeType * node,
    const std::string& topic,
    const rmw_qos_profile_t qos,
    rclcpp::SubscriptionOptions options) override
  {
    unsubscribe();

    if (!topic.empty())
    {
      topic_ = topic;
      rclcpp::QoS rclcpp_qos(rclcpp::QoSInitialization::from_rmw(qos));
      rclcpp_qos.get_rmw_qos_profile() = qos;
      qos_ = qos;
      options_ = options;
      sub_ = node->template create_subscription<M>(topic, rclcpp_qos,
          [this](std::shared_ptr<MessageType const> msg) {
            this->cb(EventType(msg));
          }, options);

      node_raw_ = node;
    }
  }

  /**
   * \brief Re-subscribe to a topic.  Only works if this subscriber has previously been subscribed to a topic.
   */
  void subscribe() override
  {
    if (!topic_.empty())
    {
      if (node_raw_ != nullptr) {
        subscribe(node_raw_, topic_, qos_, options_);
      } else if (node_shared_ != nullptr) {
        subscribe(node_shared_, topic_, qos_, options_);
      }
    }
  }

  /**
   * \brief Force immediate unsubscription of this subscriber from its topic
   */
  void unsubscribe() override
  {
    sub_.reset();
  }

  std::string getTopic() const
  {
    return this->topic_;
  }

  /**
   * \brief Returns the internal rclcpp::Subscription<M>::SharedPtr object
   */
  const typename rclcpp::Subscription<M>::SharedPtr getSubscriber() const { return sub_; }

  /**
   * \brief Does nothing.  Provided so that Subscriber may be used in a message_filters::Chain
   */
  template<typename F>
  void connectInput(F& f)
  {
    (void)f;
  }

  /**
   * \brief Does nothing.  Provided so that Subscriber may be used in a message_filters::Chain
   */
  void add(const EventType& e)
  {
    (void)e;
  }

private:

  void cb(const EventType& e)
  {
    this->signalMessage(e);
  }

  typename rclcpp::Subscription<M>::SharedPtr sub_;

  NodePtr node_shared_;
  NodeType * node_raw_ {nullptr};

  std::string topic_;
  rmw_qos_profile_t qos_;
  rclcpp::SubscriptionOptions options_;
};

}  // namespace message_filters

#endif  // MESSAGE_FILTERS__SUBSCRIBER_H_
