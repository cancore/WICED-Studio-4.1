//
//  ViewController.swift
//  WicedWiFiIntroducer
//
//  Created by bluth on 22/03/16.
//  Copyright © 2016 bluth. All rights reserved.
//

import UIKit

class ViewController: UIViewController, ScanViewControllerDelegate, WirelessSetupViewControllerDelegate, SetUpFailureViewControllerDelegate {

    @IBOutlet weak var viewcontainer: UIView!
    weak var currentViewController: UIViewController?

    override func viewDidLoad() {
        let tempController: ScanningConfigureViewController = self.storyboard?.instantiateViewControllerWithIdentifier("ComponentA") as! ScanningConfigureViewController

        tempController.data = true
        tempController.delegate = self;
        self.currentViewController = tempController

        self.addChildViewController(self.currentViewController!)

        self.currentViewController?.view.frame = CGRectMake(0,0,viewcontainer.frame.size.width, viewcontainer.frame.size.height)


        self.addSubview(self.currentViewController!.view, toView: self.viewcontainer)
        super.viewDidLoad()
    }

    func buttonAction(sender:UIButton!)
    {
        print("Button tapped")
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    func addSubview(subView:UIView, toView parentView:UIView) {
        parentView.addSubview(subView)

        var viewBindingsDict = [String: AnyObject]()
        viewBindingsDict["subView"] = subView
        parentView.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("H:|[subView]|",
            options: [], metrics: nil, views: viewBindingsDict))
        parentView.addConstraints(NSLayoutConstraint.constraintsWithVisualFormat("V:|[subView]|",
            options: [], metrics: nil, views: viewBindingsDict))
    }

     func showComponent(value: NSInteger) {
        if value == 0 {

            let newViewController:ScanningConfigureViewController = self.storyboard?.instantiateViewControllerWithIdentifier("ComponentA") as! ScanningConfigureViewController
            self.cycleFromViewController(self.currentViewController!, toViewController: newViewController)
            newViewController.data = true
            newViewController.delegate = self;
            self.currentViewController = newViewController
            
        } else if  value == 1{

            let newViewController:WirelessSetupViewController = self.storyboard?.instantiateViewControllerWithIdentifier("ComponentB") as! WirelessSetupViewController
            self.cycleFromViewController(self.currentViewController!, toViewController: newViewController)
            newViewController.delegate = self;
            self.currentViewController = newViewController

        } else if value == 2 {

            let newViewController:SetUpSuccessViewController = self.storyboard?.instantiateViewControllerWithIdentifier("ComponentC") as! SetUpSuccessViewController
            self.cycleFromViewController(self.currentViewController!, toViewController: newViewController)
            self.currentViewController = newViewController

        } else if value == 3 {

            let newViewController:SetUpFailureViewController = self.storyboard?.instantiateViewControllerWithIdentifier("ComponentD") as! SetUpFailureViewController
            self.cycleFromViewController(self.currentViewController!, toViewController: newViewController)
            newViewController.delegate = self;
            self.currentViewController = newViewController

        }
    }

    func cycleFromViewController(oldViewController: UIViewController, toViewController newViewController: UIViewController) {
        oldViewController.willMoveToParentViewController(nil)
        self.addChildViewController(newViewController)
        newViewController.view.frame = CGRectMake(0,0,viewcontainer.frame.size.width, viewcontainer.frame.size.height)
        self.addSubview(newViewController.view, toView:self.viewcontainer!)
        newViewController.view.alpha = 0
        newViewController.view.layoutIfNeeded()


        UIView.animateWithDuration(2, animations: {
            newViewController.view.alpha = 1
            oldViewController.view.alpha = 0
            },
            completion: { finished in
               oldViewController.view.removeFromSuperview()
                oldViewController.removeFromParentViewController()
               newViewController.didMoveToParentViewController(self)

        })
    }

    func scanningComplete(data: Bool) {

        print("yeeeehhh .. I reached scanningComplete")
        showComponent(1)

    }
    func setUpComplete(data: Bool) {

        print("yeeeehhh .. I reached setUpComplete")
        dispatch_async(dispatch_get_main_queue(), {
            if data {
                self.showComponent(2)
            } else {
                self.showComponent(3)
            }
        })

        
    }

    func tryAgain() {
        print("yeeeehhh .. I reached tryAgain")
        dispatch_async(dispatch_get_main_queue(), {
            self.showComponent(1)
        })

    }

}

